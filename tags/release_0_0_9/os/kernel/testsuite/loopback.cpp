/*
 * Copyright (c) 2006
 * Nintendo Co., Ltd.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Nintendo makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#include <stdlib.h>
#include <string.h>
#include <es.h>
#include <es/ref.h>
#include <es/clsid.h>
#include <es/exception.h>
#include <errno.h>
#include <es/handle.h>
#include "core.h"

#include "loopback.h"

#define TEST(exp)                           \
    (void) ((exp) ||                        \
            (esPanic(__FILE__, __LINE__, "\nFailed test " #exp), 0))

#define BUF_SIZE  (64 * 1024)

static u8 Data[BUF_SIZE * 2];
static u8 Buf[BUF_SIZE * 2];

Handle<IContext> Context;

static void* readData(void* param)
{
    Handle<IStream> stream = Context->lookup("device/loopback");
    int ret = stream->read(Buf, sizeof(Buf));
    TEST(0 < ret);
    return 0;
}

static void* writeData(void* param)
{
    Handle<IStream> stream = Context->lookup("device/loopback");
    int ret = stream->write(Data, 1);
    TEST(ret == 1);
    return 0;
}

int main()
{
    IInterface* root = NULL;
    esInit(&root);

    Context = root;
    TEST(Context);

    Handle<IStream> stream = Context->lookup("device/loopback");
    TEST(stream);

    int count = sizeof(Data);
    while (count--)
    {
        Data[count] = count % 0xff;
    }

    IThread* thread = new Thread(writeData, // thread function
                                 0,         // argument to thread function
                                 IThread::Normal - 1); // priority
    thread->start();

    // check blocking feature.
    int ret = stream->read(Buf, sizeof(Buf));
    TEST(ret == 1);

    TEST(memcmp(Buf, Data, 1) == 0);

    void* val;
    thread->join(&val);
    thread->release();

    // fill the buffer.
    int hlen = sizeof(int); // A header consumes sizeof(int) bytes.
    int size1 = 60;
    int size2 = BUF_SIZE - size1 - 2 * hlen;

    ret = stream->write(Data, size1);
    TEST(ret == size1);

    ret = stream->write(Data+64, size2);
    TEST(ret == size2);

    TEST(stream->getSize() == BUF_SIZE); // filled?

    // FIFO?
    memset(Buf, 0, sizeof(Buf));
    ret = stream->read(Buf, sizeof(Buf));
    TEST(ret == size1);
    TEST(memcmp(Buf, Data, size1) == 0);

    thread = new Thread(readData,  // thread function
                        0,         // argument to thread function
                        IThread::Normal-1); // priority
    thread->start();

    // check blocking feature.
    ret = stream->write(Data, size1 + 1);
    TEST(ret == size1 + 1);

    thread->join(&val);
    thread->release();

    // read all data.
    ret = stream->read(Buf, sizeof(Buf));
    TEST(ret == size1 + 1);

    TEST(stream->getSize() == 0); // empty?

    esReport("done.\n");
}