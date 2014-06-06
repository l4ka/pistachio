#include "builtin.hpp"
#include "hello.hpp"

namespace __hello__ {

str *const_0;


str *__name__;



void __init() {
    const_0 = new str("Hello");

    __name__ = new str("__main__");

    print2(NULL,0,1, const_0);
}

} // module namespace

int main(int, char **) {
    __shedskin__::__init();
    __shedskin__::__start(__hello__::__init);
}
