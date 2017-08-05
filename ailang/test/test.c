#define CRTDBG_MAP_ALLOC

#include "../ailang.h"
#include <crtdbg.h>

int main() {
    AiThreadState *t = AiThreadState_New(AiInterpreterState_New());
    AiThreadState_Swap(t);

    AiInt_Init();



    AiInt_ClearAllMemory();
    AiList_ClearAllMemory();
    AiDict_ClearAllMemory();

    t = AiThreadState_Swap(NULL);
    AiInterpreterState_Delete(t->interp);

    assert(heaphead == NULL);

    _CrtDumpMemoryLeaks();
    return 0;
}