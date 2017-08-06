#define CRTDBG_MAP_ALLOC

#include "../ailang.h"
#include <crtdbg.h>

int main() {
    AiThreadState *t = AiThreadState_New(AiInterpreterState_New());
    AiThreadState_Swap(t);

    AiInt_Init();

    AiType_Ready(&AiType_Int);

    AiWrapperDescrObject *w = (AiWrapperDescrObject *)
        AiDict_GetItem((AiDictObject *)AiType_Int.tp_dict, AiString_From_String("__cmp__"));

    AiInt_ClearAllMemory();
    AiList_ClearAllMemory();
    AiDict_ClearAllMemory();

    t = AiThreadState_Swap(NULL);
    AiInterpreterState_Delete(t->interp);

    _CrtDumpMemoryLeaks();
    return 0;
}