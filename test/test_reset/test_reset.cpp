// Pedidos de reset pela página (src/logica/reset.h).
#include <unity.h>

#include "logica/reset.h"

void setUp() {}
void tearDown() {}

void test_reset_confirmado_com_consoles_desligados()
{
    TEST_ASSERT_NULL(validarReset(true, 0));
}

void test_reset_sem_confirmacao_e_recusado()
{
    TEST_ASSERT_EQUAL_STRING("Confirmação ausente", validarReset(false, 0));
}

void test_reset_com_console_ligado_e_recusado()
{
    TEST_ASSERT_NOT_NULL(validarReset(true, 1 << 7));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_reset_confirmado_com_consoles_desligados);
    RUN_TEST(test_reset_sem_confirmacao_e_recusado);
    RUN_TEST(test_reset_com_console_ligado_e_recusado);
    return UNITY_END();
}
