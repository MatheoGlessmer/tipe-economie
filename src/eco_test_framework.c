#include "eco_test_framework.h"

#include <setjmp.h>
#include <signal.h>
#include <string.h>

static jmp_buf eco_test_jump_buffer;
static volatile sig_atomic_t eco_test_last_signal = 0;

static void eco_test_signal_handler(int signal_number)
{
    eco_test_last_signal = signal_number;
    longjmp(eco_test_jump_buffer, 1);
}

EcoTestResult eco_test_make_result(EcoTestStatus status,
                                   const char *message,
                                   const char *file,
                                   int line)
{
    EcoTestResult result;
    result.status = status;
    result.message = message;
    result.file = file;
    result.line = line;
    return result;
}

const char *eco_test_status_name(EcoTestStatus status)
{
    switch (status) {
        case ECO_TEST_STATUS_PASS:
            return "PASS";
        case ECO_TEST_STATUS_FAIL:
            return "FAIL";
        case ECO_TEST_STATUS_TODO:
            return "TODO";
        case ECO_TEST_STATUS_CRASH:
            return "CRASH";
        default:
            return "UNKNOWN";
    }
}

const char *eco_test_status_color(EcoTestStatus status)
{
    switch (status) {
        case ECO_TEST_STATUS_PASS:
            return ECO_TEST_COLOR_GREEN;
        case ECO_TEST_STATUS_FAIL:
            return ECO_TEST_COLOR_RED;
        case ECO_TEST_STATUS_TODO:
            return ECO_TEST_COLOR_YELLOW;
        case ECO_TEST_STATUS_CRASH:
            return ECO_TEST_COLOR_MAGENTA;
        default:
            return ECO_TEST_COLOR_RESET;
    }
}

static void eco_test_update_summary(EcoTestSummary *summary, EcoTestStatus status)
{
    if (summary == NULL) {
        return;
    }

    summary->total++;
    switch (status) {
        case ECO_TEST_STATUS_PASS:
            summary->passed++;
            break;
        case ECO_TEST_STATUS_FAIL:
            summary->failed++;
            break;
        case ECO_TEST_STATUS_TODO:
            summary->todo++;
            break;
        case ECO_TEST_STATUS_CRASH:
            summary->crashed++;
            break;
        default:
            summary->failed++;
            break;
    }
}

static EcoTestResult eco_test_run_one(const EcoTestCase *test)
{
    void (*old_sigsegv)(int) = signal(SIGSEGV, eco_test_signal_handler);
    void (*old_sigfpe)(int) = signal(SIGFPE, eco_test_signal_handler);
    void (*old_sigabrt)(int) = signal(SIGABRT, eco_test_signal_handler);
    void (*old_sigill)(int) = signal(SIGILL, eco_test_signal_handler);
#ifdef SIGBUS
    void (*old_sigbus)(int) = signal(SIGBUS, eco_test_signal_handler);
#endif

    eco_test_last_signal = 0;

    if (setjmp(eco_test_jump_buffer) != 0) {
        signal(SIGSEGV, old_sigsegv);
        signal(SIGFPE, old_sigfpe);
        signal(SIGABRT, old_sigabrt);
        signal(SIGILL, old_sigill);
#ifdef SIGBUS
        signal(SIGBUS, old_sigbus);
#endif
        return eco_test_make_result(ECO_TEST_STATUS_CRASH,
                                    "le test a déclenché un signal",
                                    __FILE__,
                                    (int)eco_test_last_signal);
    }

    EcoTestResult result = test->function();

    signal(SIGSEGV, old_sigsegv);
    signal(SIGFPE, old_sigfpe);
    signal(SIGABRT, old_sigabrt);
    signal(SIGILL, old_sigill);
#ifdef SIGBUS
    signal(SIGBUS, old_sigbus);
#endif

    return result;
}

int eco_test_run_all(const EcoTestCase *tests,
                     size_t test_count,
                     bool use_color,
                     EcoTestSummary *summary)
{
    if (summary != NULL) {
        memset(summary, 0, sizeof(*summary));
    }

    printf("%sSuite de tests du modèle économique%s\n",
           use_color ? ECO_TEST_COLOR_BOLD : "",
           use_color ? ECO_TEST_COLOR_RESET : "");
    printf("%zu test(s) déclaré(s)\n\n", test_count);

    for (size_t i = 0; i < test_count; ++i) {
        EcoTestResult result = eco_test_run_one(&tests[i]);
        eco_test_update_summary(summary, result.status);

        const char *color = use_color ? eco_test_status_color(result.status) : "";
        const char *reset = use_color ? ECO_TEST_COLOR_RESET : "";

        printf("%s[%s]%s %s :: test %zu - %s",
               color,
               eco_test_status_name(result.status),
               reset,
               tests[i].suite_name,
               i + 1,
               tests[i].test_name);

        if (result.message != NULL) {
            printf(" — %s", result.message);
        }

        if (result.status == ECO_TEST_STATUS_FAIL) {
            printf(" (%s:%d)", result.file, result.line);
        } else if (result.status == ECO_TEST_STATUS_CRASH) {
            printf(" (signal %d)", result.line);
        }

        printf("\n");
    }

    if (summary != NULL) {
        printf("\n%sRésumé%s : ",
               use_color ? ECO_TEST_COLOR_BOLD : "",
               use_color ? ECO_TEST_COLOR_RESET : "");
        printf("%s%zu réussi(s)%s, ",
               use_color ? ECO_TEST_COLOR_GREEN : "",
               summary->passed,
               use_color ? ECO_TEST_COLOR_RESET : "");
        printf("%s%zu échoué(s)%s, ",
               use_color ? ECO_TEST_COLOR_RED : "",
               summary->failed,
               use_color ? ECO_TEST_COLOR_RESET : "");
        printf("%s%zu à implémenter%s, ",
               use_color ? ECO_TEST_COLOR_YELLOW : "",
               summary->todo,
               use_color ? ECO_TEST_COLOR_RESET : "");
        printf("%s%zu planté(s)%s.\n",
               use_color ? ECO_TEST_COLOR_MAGENTA : "",
               summary->crashed,
               use_color ? ECO_TEST_COLOR_RESET : "");
    }

    if (summary != NULL && (summary->failed > 0 || summary->crashed > 0)) {
        return 1;
    }

    return 0;
}
