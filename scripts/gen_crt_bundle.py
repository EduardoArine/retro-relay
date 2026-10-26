# Gera certs/x509_crt_bundle.bin: os certificados raiz usados para validar o
# HTTPS do OTA (GitHub). Formato lido por WiFiClientSecure::setCACertBundle()
# do core Arduino 2.0.x (esp_crt_bundle.c):
#
#   [2 bytes: quantidade]
#   para cada certificado, ordenados pelo subject (o core faz busca binária):
#     [2 bytes: tamanho do subject][2 bytes: tamanho da chave]
#     [subject DER][chave pública DER (SubjectPublicKeyInfo)]
#
# Uso (rodar de vez em quando para atualizar a lista de raízes):
#   python scripts/gen_crt_bundle.py              # baixa a lista da Mozilla (curl.se)
#   python scripts/gen_crt_bundle.py cacert.pem   # usa um arquivo local
#
# Só biblioteca padrão: roda no Python do PlatformIO sem instalar nada.

import base64
import os
import re
import struct
import sys
import urllib.request

FONTE = "https://curl.se/ca/cacert.pem"  # Raízes confiáveis da Mozilla para TLS
SAIDA = os.path.join(os.path.dirname(__file__), "..", "certs", "x509_crt_bundle.bin")


def ler_tlv(der, pos):
    """Retorna (tag, início do conteúdo, fim do TLV)."""
    tag = der[pos]
    tamanho = der[pos + 1]
    pos += 2
    if tamanho & 0x80:
        n = tamanho & 0x7F
        tamanho = int.from_bytes(der[pos:pos + n], "big")
        pos += n
    return tag, pos, pos + tamanho


def campos_tbs(der):
    """TLVs brutos dos campos do TBSCertificate, sem a versão opcional."""
    _, conteudo_cert, _ = ler_tlv(der, 0)          # Certificate
    _, pos, fim_tbs = ler_tlv(der, conteudo_cert)  # TBSCertificate
    campos = []
    while pos < fim_tbs:
        inicio = pos
        _, _, pos = ler_tlv(der, pos)
        campos.append(der[inicio:pos])
    if campos[0][0] == 0xA0:  # [0] version
        campos = campos[1:]
    return campos  # serial, algoritmo, issuer, validade, subject, chave, ...


def subject_e_chave(der):
    campos = campos_tbs(der)
    return campos[4], campos[5]


def ler_pems(texto):
    blocos = re.findall(
        r"-----BEGIN CERTIFICATE-----(.+?)-----END CERTIFICATE-----", texto, re.S
    )
    return [base64.b64decode("".join(b.split())) for b in blocos]


def main():
    if len(sys.argv) > 1:
        with open(sys.argv[1], encoding="utf-8") as f:
            texto = f.read()
    else:
        with urllib.request.urlopen(FONTE) as r:
            texto = r.read().decode("utf-8")

    certificados = sorted(subject_e_chave(der) for der in ler_pems(texto))

    saida = bytearray(struct.pack(">H", len(certificados)))
    for subject, chave in certificados:
        saida += struct.pack(">HH", len(subject), len(chave)) + subject + chave

    os.makedirs(os.path.dirname(SAIDA), exist_ok=True)
    with open(SAIDA, "wb") as f:
        f.write(saida)
    print(f"{len(certificados)} certificados, {len(saida)} bytes -> {os.path.normpath(SAIDA)}")


if __name__ == "__main__":
    main()
