# Confere o certs/x509_crt_bundle.bin embutido no firmware: se ele estiver
# corrompido ou sem as raízes do GitHub, o OTA deixa de funcionar.
# Rodar: python -m unittest discover -s scripts/tests
import os
import sys
import unittest

RAIZ = os.path.join(os.path.dirname(__file__), "..", "..")
sys.path.insert(0, os.path.join(RAIZ, "scripts"))
import gen_crt_bundle  # noqa: E402

BUNDLE = os.path.join(RAIZ, "certs", "x509_crt_bundle.bin")

# Raízes das cadeias servidas hoje pelos hosts do OTA
RAIZES_GITHUB = [
    b"USERTrust ECC Certification Authority",  # api.github.com, github.com
    b"ISRG Root X1",                           # release-assets.githubusercontent.com
]


def ler_bundle():
    with open(BUNDLE, "rb") as f:
        dados = f.read()
    quantidade = int.from_bytes(dados[:2], "big")
    entradas, pos = [], 2
    for _ in range(quantidade):
        tam_nome = int.from_bytes(dados[pos:pos + 2], "big")
        tam_chave = int.from_bytes(dados[pos + 2:pos + 4], "big")
        inicio = pos + 4
        entradas.append((dados[inicio:inicio + tam_nome], dados[inicio + tam_nome:inicio + tam_nome + tam_chave]))
        pos = inicio + tam_nome + tam_chave
    return dados, quantidade, entradas, pos


class TestCrtBundle(unittest.TestCase):
    def setUp(self):
        self.dados, self.quantidade, self.entradas, self.lido = ler_bundle()

    def test_tamanho_bate_com_o_cabecalho(self):
        self.assertGreater(self.quantidade, 50)
        self.assertEqual(self.lido, len(self.dados))

    def test_ordenado_pelo_subject(self):
        # O core faz busca binária pelo subject
        nomes = [nome for nome, _ in self.entradas]
        self.assertEqual(nomes, sorted(nomes))

    def test_entradas_sao_der_valido(self):
        for nome, chave in self.entradas:
            self.assertEqual(nome[0], 0x30)   # Name: SEQUENCE
            self.assertEqual(chave[0], 0x30)  # SubjectPublicKeyInfo: SEQUENCE
            for tlv in (nome, chave):
                _, _, fim = gen_crt_bundle.ler_tlv(tlv, 0)
                self.assertEqual(fim, len(tlv))

    def test_contem_as_raizes_do_github(self):
        for raiz in RAIZES_GITHUB:
            self.assertTrue(any(raiz in nome for nome, _ in self.entradas), raiz.decode())


if __name__ == "__main__":
    unittest.main()
