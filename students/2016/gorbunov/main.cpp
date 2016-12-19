#include "mathvm.h"

#include <fcntl.h>

using namespace mathvm;
using namespace std;

int main(int argc, char **argv) {
    string impl = "translator";
    const char *script = NULL;
    bool print_program = false;
    bool print_bytecode = false;
    for (int32_t i = 1; i < argc; i++) {
        if (string(argv[i]) == "-j") {
            impl = "jit";
        } else if (string(argv[i]) == "-p") {
            impl = "printer";
        } else if (string(argv[i]) == "-t") {
            impl = "translator";
        } else if (string(argv[i]) == "--print-program") {
            print_program = true;
        } else if (string(argv[i]) == "--bytecode") {
            print_bytecode = true;
        } else {
            script = argv[i];
        }
    }
    Translator *translator = Translator::create(impl);

    if (translator == 0) {
        cout << "TODO: Implement translator factory in translator.cpp!!!!" << endl;
        return 1;
    }

    const char *expr = "double x; double y;"
            "x += 8.0; y = 2.0;"
            "print('Hello, x=',x,' y=',y,'\n');";
    bool isDefaultExpr = true;

    if (script != NULL) {
        expr = loadFile(script);
        if (expr == 0) {
            printf("Cannot read file: %s\n", script);
            return 1;
        }
        isDefaultExpr = false;
    }

    Code *code = 0;

    if (print_program) {
        auto p = Translator::create("printer");
        p->translate(expr, nullptr);
        delete p;
    }

    Status *translateStatus = translator->translate(expr, &code);
    if (translateStatus->isError()) {
        uint32_t position = translateStatus->getPosition();
        uint32_t line = 0, offset = 0;
        positionToLineOffset(expr, position, line, offset);
        printf("Cannot translate expression: expression at %d,%d; "
                       "error '%s'\n",
               line, offset,
               translateStatus->getErrorCstr());
    } else {
        if (impl == "translator" && print_bytecode) {
            code->disassemble();
        }
        if (impl != "printer") {
            assert(code != 0);
            vector<Var *> vars;

            if (isDefaultExpr) {
                Var *xVar = new Var(VT_DOUBLE, "x");
                Var *yVar = new Var(VT_DOUBLE, "y");
                vars.push_back(xVar);
                vars.push_back(yVar);
                xVar->setDoubleValue(42.0);
            }
            Status *execStatus = code->execute(vars);
            if (execStatus->isError()) {
                printf("Cannot execute expression: error: %s\n",
                       execStatus->getErrorCstr());
            } else {
                if (isDefaultExpr) {
                    printf("x evaluated to %f\n", vars[0]->getDoubleValue());
                    for (uint32_t i = 0; i < vars.size(); i++) {
                        delete vars[i];
                    }
                }
            }
            delete code;
            delete execStatus;
        }
    }
    delete translateStatus;
    delete translator;

    if (!isDefaultExpr) {
        delete[] expr;
    }

    return 0;
}