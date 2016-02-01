/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Pool

FILE DESCRIPTION
Pools of fixed-size buffers allocated from the relocatable heap. Currently
unused buffers are kept unlocked and unretained (so they can be discarded by the
memory manager when we get tight on memory) in a linked list.
=============================================================================*/
