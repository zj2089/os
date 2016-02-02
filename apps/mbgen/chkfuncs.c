/*++

Copyright (c) 2016 Minoca Corp. All Rights Reserved

Module Name:

    chkfuncs.c

Abstract:

    This module implements the functions built-in to Chalk for the mbgen
    program.

Author:

    Evan Green 15-Jan-2016

Environment:

    POSIX

--*/

//
// ------------------------------------------------------------------- Includes
//

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbgen.h"

//
// ---------------------------------------------------------------- Definitions
//

//
// ------------------------------------------------------ Data Type Definitions
//

//
// ----------------------------------------------- Internal Function Prototypes
//

INT
MbgenChalkAssert (
    PCHALK_INTERPRETER Interpreter,
    PVOID Context,
    PCHALK_OBJECT *ReturnValue
    );

INT
MbgenChalkSplitExtension (
    PCHALK_INTERPRETER Interpreter,
    PVOID Context,
    PCHALK_OBJECT *ReturnValue
    );

//
// -------------------------------------------------------------------- Globals
//

PSTR MbgenChalkAssertArguments[] = {
    "condition",
    "complaint",
    NULL
};

PSTR MbgenChalkSplitExtensionArguments[] = {
    "path",
    NULL
};

CHALK_FUNCTION_PROTOTYPE MbgenChalkFunctions[] = {
    {
        "assert",
        MbgenChalkAssertArguments,
        MbgenChalkAssert
    },

    {
        "split_extension",
        MbgenChalkSplitExtensionArguments,
        MbgenChalkSplitExtension
    },

    {NULL}
};

//
// ------------------------------------------------------------------ Functions
//

INT
MbgenAddChalkBuiltins (
    PMBGEN_CONTEXT Context
    )

/*++

Routine Description:

    This routine adds the functions in the global scope of the Chalk
    interpreter for the mbgen program.

Arguments:

    Context - Supplies a pointer to the application context.

Return Value:

    0 on success.

    Non-zero on failure.

--*/

{

    INT Status;

    Status = ChalkRegisterFunctions(&(Context->Interpreter),
                                    Context,
                                    MbgenChalkFunctions);

    return Status;
}

//
// --------------------------------------------------------- Internal Functions
//

INT
MbgenChalkAssert (
    PCHALK_INTERPRETER Interpreter,
    PVOID Context,
    PCHALK_OBJECT *ReturnValue
    )

/*++

Routine Description:

    This routine implements the assert function for Chalk in the mbgen context.

Arguments:

    Interpreter - Supplies a pointer to the interpreter context.

    Context - Supplies a pointer's worth of context given when the function
        was registered.

    ReturnValue - Supplies a pointer where a pointer to the return value will
        be returned.

Return Value:

    0 on success.

    Returns an error number on execution failure.

--*/

{

    PCHALK_OBJECT Condition;

    *ReturnValue = NULL;
    Condition = ChalkCGetVariable(Interpreter, "condition");

    assert(Condition != NULL);

    if (ChalkObjectGetBooleanValue(Condition) != FALSE) {
        return 0;
    }

    fprintf(stderr, "Assertion failure: ");
    ChalkPrintObject(stderr, ChalkCGetVariable(Interpreter, "complaint"), 0);
    return EINVAL;
}

INT
MbgenChalkSplitExtension (
    PCHALK_INTERPRETER Interpreter,
    PVOID Context,
    PCHALK_OBJECT *ReturnValue
    )

/*++

Routine Description:

    This routine implements the split_extension Chalk function.

Arguments:

    Interpreter - Supplies a pointer to the interpreter context.

    Context - Supplies a pointer's worth of context given when the function
        was registered.

    ReturnValue - Supplies a pointer where a pointer to the return value will
        be returned.

Return Value:

    0 on success.

    Returns an error number on execution failure.

--*/

{

    PCHALK_OBJECT Array[2];
    PSTR Base;
    PCHALK_OBJECT BaseString;
    PSTR Extension;
    PCHALK_OBJECT ExtensionString;
    PCHALK_OBJECT List;
    PCHALK_OBJECT Path;
    INT Status;

    Base = NULL;
    BaseString = NULL;
    ExtensionString = NULL;
    *ReturnValue = NULL;
    Path = ChalkCGetVariable(Interpreter, "path");

    assert(Path != NULL);

    if (Path->Header.Type != ChalkObjectString) {
        fprintf(stderr, "split_extension: String expected\n");
        return EINVAL;
    }

    Extension = NULL;
    Base = MbgenSplitExtension(Path->String.String, &Extension);
    if (Base == NULL) {
        BaseString = ChalkCreateString("", 0);

    } else {
        BaseString = ChalkCreateString(Base, strlen(Base));
    }

    if (BaseString == NULL) {
        Status = ENOMEM;
        goto ChalkSplitExtensionEnd;
    }

    if (Extension == NULL) {
        ExtensionString = ChalkCreateString("", 0);

    } else {
        ExtensionString = ChalkCreateString(Extension, strlen(Extension));
    }

    if (ExtensionString == NULL) {
        Status = ENOMEM;
        goto ChalkSplitExtensionEnd;
    }

    Array[0] = BaseString;
    Array[1] = ExtensionString;
    List = ChalkCreateList(Array, 2);
    if (List == NULL) {
        Status = ENOMEM;
        goto ChalkSplitExtensionEnd;
    }

    *ReturnValue = List;
    Status = 0;

ChalkSplitExtensionEnd:
    if (Base != NULL) {
        free(Base);
    }

    if (BaseString != NULL) {
        ChalkObjectReleaseReference(BaseString);
    }

    if (ExtensionString != NULL) {
        ChalkObjectReleaseReference(ExtensionString);
    }

    return Status;
}
