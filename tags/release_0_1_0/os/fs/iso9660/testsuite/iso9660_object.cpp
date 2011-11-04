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

#include <new>
#include <stdlib.h>
#include <es.h>
#include <es/exception.h>
#include <es/handle.h>
#include "vdisk.h"
#include "iso9660Stream.h"

#define TEST(exp)                           \
    (void) ((exp) ||                        \
            (esPanic(__FILE__, __LINE__, "\nFailed test " #exp), 0))

void test(Handle<IContext> root)
{
    Handle<IIterator>   iter;
    Handle<IFile>       file;
    Handle<IStream>     stream;
    long long           size = 0;

    // List the root directory
    iter = root->list("");
    while (iter->hasNext())
    {
        char name[1024];
        Handle<IBinding> binding(iter->next());
        binding->getName(name, sizeof name);
#ifdef VERBOSE
        esReport("'%s'\n", name);
#endif // VERBOSE
    }

    // Read
    file = root->lookup("NOTICE");
    TEST(file);
    size = file->getSize();
#ifdef VERBOSE
    esReport("Size: %lld\n", size);
#endif // VERBOSE
    u8* buf = new u8[size];
    stream = file->getStream();
    TEST(stream);
    stream->read(buf, size);
#ifdef VERBOSE
    esReport("%.*s\n", (int) size, buf);
#endif // VERBOSE
    delete [] buf;
}

int main(int argc, char* argv[])
{
    IInterface* ns = 0;
    esInit(&ns);
    Handle<IContext> nameSpace(ns);

    Handle<IClassStore> classStore(nameSpace->lookup("class"));
    esRegisterIsoFileSystemClass(classStore);

#ifdef __es__
    Handle<IStream> disk = nameSpace->lookup("device/ata/channel1/device0");
#else
    IStream* disk = new VDisk(static_cast<char*>("isotest.iso"));
#endif
    TEST(disk);
    long long diskSize;
    diskSize = disk->getSize();
    esReport("diskSize: %lld\n", diskSize);
    TEST(0 < diskSize);

    Handle<IFileSystem> isoFileSystem;
    isoFileSystem = reinterpret_cast<IFileSystem*>(
        esCreateInstance(CLSID_IsoFileSystem, IFileSystem::iid()));
    TEST(isoFileSystem);
    isoFileSystem->mount(disk);
    {
        Handle<IContext> root;

        root = isoFileSystem->getRoot();
        TEST(root);

        Handle<IBinding> binding = root;
        TEST(binding);

        Handle<IInterface> interface = binding->getObject();
        TEST(interface);

        Handle<IContext> object = interface;
        TEST(object);
        TEST(object == root);

        // setObject() must return an exception.
        try
        {
            binding->setObject(interface);
            TEST(false);
        }
        catch (Exception& error)
        {
        }
    }
    isoFileSystem->dismount();

    esReport("done.\n");
}