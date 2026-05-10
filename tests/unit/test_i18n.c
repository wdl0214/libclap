/**
 * @file test_i18n.c
 * @brief Unit tests for i18n (translation) support
 */

#include "unity.h"
#include "clap_parser_internal.h"
#include <string.h>

/* ============================================================================
 * Test Helpers — a simple French translator for testing
 * ============================================================================ */

static const char* french_translate(const char *msgid, void *user_data) {
    (void)user_data;
    /* Return translations for a subset of strings — keep English for others */
    if (strcmp(msgid, "Success") == 0) return "Succès";
    if (strcmp(msgid, "Invalid argument") == 0) return "Argument invalide";
    if (strcmp(msgid, "Type conversion failed") == 0) return "Échec de conversion de type";
    if (strcmp(msgid, "Memory allocation failed") == 0) return "Échec d'allocation mémoire";
    if (strcmp(msgid, "Unknown error") == 0) return "Erreur inconnue";
    if (strcmp(msgid, "Usage: ") == 0) return "Utilisation: ";
    if (strcmp(msgid, "Positional arguments:") == 0) return "Arguments positionnels:";
    if (strcmp(msgid, "Optional arguments:") == 0) return "Arguments optionnels:";
    if (strcmp(msgid, "Commands:") == 0) return "Commandes:";
    if (strcmp(msgid, "unknown") == 0) return "inconnu";
    if (strcmp(msgid, "Show this help message and exit") == 0)
        return "Afficher ce message d'aide et quitter";
    if (strcmp(msgid, "error") == 0) return "erreur";
    if (strcmp(msgid, "warning") == 0) return "avertissement";
    if (strcmp(msgid, " [-h]") == 0) return " [-a]";
    return msgid;  /* fall back to English */
}

/* ============================================================================
 * Test Setup and Teardown
 * ============================================================================ */

void setUp(void) {
    /* Nothing needed */
}

void tearDown(void) {
    /* Nothing needed */
}

/* ============================================================================
 * clap_set_translator / clap_i18n_translate Tests
 * ============================================================================ */

void test_i18n_translate_strerror(void) {
    /* Set a French translator */
    clap_set_translator(french_translate, NULL);

    /* Verify clap_strerror returns translated strings */
    TEST_ASSERT_EQUAL_STRING("Succès", clap_strerror(CLAP_ERR_NONE));
    TEST_ASSERT_EQUAL_STRING("Argument invalide", clap_strerror(CLAP_ERR_INVALID_ARGUMENT));
    TEST_ASSERT_EQUAL_STRING("Échec de conversion de type",
                              clap_strerror(CLAP_ERR_TYPE_CONVERSION));
    TEST_ASSERT_EQUAL_STRING("Échec d'allocation mémoire",
                              clap_strerror(CLAP_ERR_MEMORY));
    TEST_ASSERT_EQUAL_STRING("Erreur inconnue", clap_strerror(-1));

    /* Verify untranslated code still falls back to English */
    TEST_ASSERT_EQUAL_STRING("Missing required value",
                              clap_strerror(CLAP_ERR_MISSING_VALUE));
}

void test_i18n_translate_unknown_code(void) {
    clap_set_translator(french_translate, NULL);
    TEST_ASSERT_EQUAL_STRING("Erreur inconnue", clap_strerror(9999));
}

/* ============================================================================
 * Formatter output with translator
 * ============================================================================ */

/* Helper: capture output from a function that writes to a FILE* */
static size_t capture_output(void (*func)(clap_parser_t*, FILE*),
                              clap_parser_t *parser,
                              char *buf, size_t buf_size) {
    FILE *fp = tmpfile();
    if (!fp) return 0;

    func(parser, fp);
    rewind(fp);

    size_t total = fread(buf, 1, buf_size - 1, fp);
    buf[total] = '\0';
    fclose(fp);
    return total;
}

void test_i18n_help_translated(void) {
    /* Set French translator */
    clap_set_translator(french_translate, NULL);

    clap_parser_t *parser = clap_parser_new("prog", "A test program", NULL);

    /* Add an argument to ensure help has content */
    clap_argument_t *arg = clap_add_argument(parser, "--verbose/-v");
    clap_argument_help(arg, "Enable verbose output");

    char buf[4096];
    capture_output(clap_print_help, parser, buf, sizeof(buf));

    /* Verify section headings are translated */
    TEST_ASSERT_NOT_NULL(strstr(buf, "Utilisation:"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Arguments optionnels:"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Afficher ce message d'aide et quitter"));

    /* Verify English originals are NOT present */
    TEST_ASSERT_NULL(strstr(buf, "Usage:"));
    TEST_ASSERT_NULL(strstr(buf, "Optional arguments:"));
    TEST_ASSERT_NULL(strstr(buf, "Show this help message and exit"));

    clap_parser_free(parser);
}

void test_i18n_version_translated(void) {
    clap_set_translator(french_translate, NULL);

    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    char buf[1024];
    capture_output(clap_print_version, parser, buf, sizeof(buf));

    /* Check the "unknown" fallback is translated */
    TEST_ASSERT_NOT_NULL(strstr(buf, "inconnu"));

    clap_parser_free(parser);
}

void test_i18n_usage_translated(void) {
    clap_set_translator(french_translate, NULL);

    clap_parser_t *parser = clap_parser_new("prog", NULL, NULL);
    char buf[1024];
    capture_output(clap_print_usage, parser, buf, sizeof(buf));

    TEST_ASSERT_NOT_NULL(strstr(buf, "Utilisation:"));
    TEST_ASSERT_NULL(strstr(buf, "Usage:"));

    clap_parser_free(parser);
}

/* Verify that error structs with NULL format fall back to translated strerror */
void test_i18n_error_set_null_format(void) {
    clap_set_translator(french_translate, NULL);

    clap_error_t error;
    clap_error_init(&error);
    clap_error_set(&error, CLAP_ERR_MEMORY, NULL);

    TEST_ASSERT_EQUAL_STRING("Échec d'allocation mémoire", error.message);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

void run_test_i18n(void) {
    /* Note: clap_set_translator is one-shot (locked after first call).
     * These tests are ordered to run in sequence, each relying on the
     * translator being already set.  Do not reorder. */

    RUN_TEST(test_i18n_translate_strerror);
    RUN_TEST(test_i18n_translate_unknown_code);
    RUN_TEST(test_i18n_help_translated);
    RUN_TEST(test_i18n_version_translated);
    RUN_TEST(test_i18n_usage_translated);
    RUN_TEST(test_i18n_error_set_null_format);
}

#ifdef STANDALONE_TEST
int main(void) {
    UNITY_BEGIN();
    run_test_i18n();
    return UNITY_END();
}
#endif
