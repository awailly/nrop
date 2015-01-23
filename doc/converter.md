---
title: nROP converters
layout: base
---

[xed]: https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads
[capstone]: http://capstone-engine.org
[xedonly]: http://ix.io/17D
[caponly]: http://ix.io/18a

# Converter

The file `converter.c` handles all operations to convert from a language to another (resp. ASM -> TCG -> LLVM -> z3).

We start with a file named `test.c` in the `src/` directory of nrop.

You need to feed the converter with a chunk (easy way) or a linked list with instructions (hard way as you have to use nrop linked lists).

    #include "chunk.h"
    #include "chain.h"
    #include "converter.h"
    #include <setjmp.h>
    #include "qemu/include/exec/exec-all.h"
    #include "qemu/tcg/tcg.h"

    TCGContext *get_tcg_ctx(void);

    int main(int argc, char *argv[])
    {
        CPUArchState *env;
        CPUState *cpu;
        TranslationBlock *tb;
        TCGContext *s;

        Z3_config cfg;
        Z3_context ctx;

        chunk_t c, target;
        converter_t *converter;
        map_t *map;

        if (argc != 2)
            printf("Need the asm bytes\n");
        
        c = chunk_calloc(strlen(argv[1]));
        memcpy(c.ptr, argv[1], c.len);
        target = chunk_from_hex(c, NULL);

        // TCG
        module_call_init(MODULE_INIT_QOM);
        tcg_exec_init(0);
        cpu_exec_init_all();
        env = cpu_init("qemu64");
        cpu = ENV_GET_CPU(env);
        tb = tb_gen_code(cpu, (uint64_t) target.ptr, 0, 0xc0c0c0, 0);

        s = get_tcg_ctx();
        tcg_dump_ops(s); // Dump TCG Here

        cfg = Z3_mk_config();
        ctx = Z3_mk_context(cfg);
        Z3_del_config(cfg);

        converter = converter_create(s, ctx); // Replace NULL with z3 context for llvm_to_z3
        converter->set_prefix(converter, chunk_create((unsigned char *)"", 1));
        converter->tcg_to_llvm(converter);
        converter->dump(converter); // Dump LLVM Here

        map = converter->llvm_to_z3(converter);
        map->destroy(map);

        Z3_del_context(ctx);

        chunk_free(&c);
        chunk_free(&target);
    }

Which mimic the implementation of the `get_map()` function.

Note: Only some operations are implemented, feel free to enhance the converters.

Compile with the magic command

    clang  -g -fsanitize=address,leak -fno-omit-frame-pointer -fno-common -DNEED_CPU_H -DCONFIG_USER_ONLY -pthread -W -Wall -Wextra  -isystem ../libs/xed2-intel64/include -I. -I../libs/ -Idisassemblers -Iplugins -Iparsers -isystem ../libs/qemu/tcg/i386 -isystem ../libs/qemu/x86_64-linux-user -isystem ../libs/qemu/target-i386 -isystem ../libs/qemu -isystem ../libs/qemu/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I../libs/z3/build -I../libs/z3/src/api -c test.c -o test.o `llvm-config --cflags`

and link (command generated from Makefile and containing useless objects)

    clang++ -fsanitize=address,leak test.o chunk.o enumerator.o linked_list.o utils.o region.o thpool.o chain.o map.o constraints.o converter.o disassemblers/disassembler_xed.o disassemblers/disassembler_capstone.o disassemblers/disassembler_meta.o parsers/section.o parsers/program_header.o parsers/parser_builder.o parsers/elf_type.o parsers/pe_type.o parsers/parser_elf.o parsers/parser_pe.o plugins/plugin_builder.o plugins/plugin_rop.o -isystem ../libs/xed2-intel64/include -I. -I../libs/ -Idisassemblers -Iplugins -Iparsers -isystem ../libs/qemu/tcg/i386 -isystem ../libs/qemu/x86_64-linux-user -isystem ../libs/qemu/target-i386 -isystem ../libs/qemu -isystem ../libs/qemu/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I../libs/z3/build -I../libs/z3/src/api -o testme ../libs/xed2-intel64/lib/libxed.a -lcapstone -Lz3/build -L/usr/lib -lz -lm -lrt -lglib-2.0 ../libs/qemu/qobject/qfloat.o ../libs/qemu/qobject/json-streamer.o ../libs/qemu/qobject/qdict.o ../libs/qemu/qobject/json-parser.o ../libs/qemu/qobject/qjson.o ../libs/qemu/qobject/qerror.o ../libs/qemu/qobject/json-lexer.o ../libs/qemu/qobject/qstring.o ../libs/qemu/qobject/qlist.o ../libs/qemu/qobject/qbool.o ../libs/qemu/qobject/qint.o ../libs/qemu/block.o ../libs/qemu/qemu-coroutine.o ../libs/qemu/trace/control.o ../libs/qemu/trace/generated-events.o ../libs/qemu/util/event_notifier-posix.o ../libs/qemu/util/path.o ../libs/qemu/util/notify.o ../libs/qemu/util/envlist.o ../libs/qemu/util/acl.o ../libs/qemu/util/unicode.o ../libs/qemu/util/hbitmap.o ../libs/qemu/util/bitmap.o ../libs/qemu/util/bitops.o ../libs/qemu/util/crc32c.o ../libs/qemu/util/qemu-sockets.o ../libs/qemu/util/compatfd.o ../libs/qemu/util/qemu-option.o ../libs/qemu/util/qemu-timer-common.o ../libs/qemu/util/aes.o ../libs/qemu/util/error.o ../libs/qemu/util/qemu-error.o ../libs/qemu/util/uri.o ../libs/qemu/util/iov.o ../libs/qemu/util/rfifolock.o ../libs/qemu/util/osdep.o ../libs/qemu/util/qemu-config.o ../libs/qemu/util/getauxval.o ../libs/qemu/util/qemu-thread-posix.o ../libs/qemu/util/module.o ../libs/qemu/util/hexdump.o ../libs/qemu/util/oslib-posix.o ../libs/qemu/util/throttle.o ../libs/qemu/util/readline.o ../libs/qemu/util/cutils.o ../libs/qemu/util/qemu-progress.o ../libs/qemu/blockjob.o ../libs/qemu/x86_64-linux-user/disas.o ../libs/qemu/x86_64-linux-user/kvm-stub.o ../libs/qemu/x86_64-linux-user/translate-all.o ../libs/qemu/x86_64-linux-user/tcg/optimize.o ../libs/qemu/x86_64-linux-user/tcg/tcg.o ../libs/qemu/x86_64-linux-user/linux-user/signal.o ../libs/qemu/x86_64-linux-user/linux-user/syscall.o ../libs/qemu/x86_64-linux-user/linux-user/mmap.o ../libs/qemu/x86_64-linux-user/linux-user/linuxload.o ../libs/qemu/x86_64-linux-user/linux-user/uaccess.o ../libs/qemu/x86_64-linux-user/linux-user/uname.o ../libs/qemu/x86_64-linux-user/linux-user/strace.o ../libs/qemu/x86_64-linux-user/linux-user/elfload.o ../libs/qemu/x86_64-linux-user/fpu/softfloat.o ../libs/qemu/x86_64-linux-user/cpu-exec.o ../libs/qemu/x86_64-linux-user/exec.o ../libs/qemu/x86_64-linux-user/gdbstub.o ../libs/qemu/x86_64-linux-user/user-exec.o ../libs/qemu/x86_64-linux-user/target-i386/excp_helper.o ../libs/qemu/x86_64-linux-user/target-i386/seg_helper.o ../libs/qemu/x86_64-linux-user/target-i386/kvm-stub.o ../libs/qemu/x86_64-linux-user/target-i386/misc_helper.o ../libs/qemu/x86_64-linux-user/target-i386/mem_helper.o ../libs/qemu/x86_64-linux-user/target-i386/helper.o ../libs/qemu/x86_64-linux-user/target-i386/ioport-user.o ../libs/qemu/x86_64-linux-user/target-i386/translate.o ../libs/qemu/x86_64-linux-user/target-i386/cpu.o ../libs/qemu/x86_64-linux-user/target-i386/smm_helper.o ../libs/qemu/x86_64-linux-user/target-i386/gdbstub.o ../libs/qemu/x86_64-linux-user/target-i386/fpu_helper.o ../libs/qemu/x86_64-linux-user/target-i386/int_helper.o ../libs/qemu/x86_64-linux-user/target-i386/cc_helper.o ../libs/qemu/x86_64-linux-user/target-i386/svm_helper.o ../libs/qemu/x86_64-linux-user/thunk.o ../libs/qemu/main-loop.o ../libs/qemu/qemu-io-cmds.o ../libs/qemu/qapi/qmp-registry.o ../libs/qemu/qapi/opts-visitor.o ../libs/qemu/qapi/qmp-input-visitor.o ../libs/qemu/qapi/qmp-event.o ../libs/qemu/qapi/qmp-dispatch.o ../libs/qemu/qapi/qmp-output-visitor.o ../libs/qemu/qapi/qapi-visit-core.o ../libs/qemu/qapi/qapi-dealloc-visitor.o ../libs/qemu/qapi/string-input-visitor.o ../libs/qemu/qapi/string-output-visitor.o ../libs/qemu/tcg-runtime.o ../libs/qemu/qapi-types.o ../libs/qemu/async.o ../libs/qemu/qemu-coroutine-io.o ../libs/qemu/qom/qom-qobject.o ../libs/qemu/qom/container.o ../libs/qemu/qom/object_interfaces.o ../libs/qemu/qom/cpu.o ../libs/qemu/qom/object.o ../libs/qemu/hw/core/qdev.o ../libs/qemu/hw/core/fw-path-provider.o ../libs/qemu/hw/core/irq.o ../libs/qemu/hw/core/hotplug.o ../libs/qemu/hw/core/qdev-properties.o ../libs/qemu/coroutine-ucontext.o ../libs/qemu/disas/i386.o ../libs/qemu/qemu-coroutine-lock.o ../libs/qemu/qapi-visit.o ../libs/qemu/qemu-coroutine-sleep.o ../libs/qemu/aio-posix.o ../libs/qemu/iohandler.o ../libs/qemu/block/qcow.o ../libs/qemu/block/qed-check.o ../libs/qemu/block/qed.o ../libs/qemu/block/qcow2-cluster.o ../libs/qemu/block/sheepdog.o ../libs/qemu/block/qcow2.o ../libs/qemu/block/nbd-client.o ../libs/qemu/block/dmg.o ../libs/qemu/block/blkdebug.o ../libs/qemu/block/qed-cluster.o ../libs/qemu/block/cloop.o ../libs/qemu/block/nbd.o ../libs/qemu/nbd.o ../libs/qemu/block/raw_bsd.o ../libs/qemu/block/qed-l2-cache.o ../libs/qemu/block/parallels.o ../libs/qemu/block/vpc.o ../libs/qemu/block/blkverify.o ../libs/qemu/block/vdi.o ../libs/qemu/block/snapshot.o ../libs/qemu/block/vmdk.o ../libs/qemu/block/qed-table.o ../libs/qemu/block/qapi.o ../libs/qemu/block/qcow2-snapshot.o ../libs/qemu/block/bochs.o ../libs/qemu/block/qed-gencb.o ../libs/qemu/block/vvfat.o ../libs/qemu/block/qcow2-refcount.o ../libs/qemu/block/qcow2-cache.o ../libs/qemu/qemu-log.o ../libs/qemu/qemu-timer.o ../libs/qemu/qapi-event.o ../libs/qemu/thread-pool.o ../libs/qemu/stubs/slirp.o ../libs/qemu/stubs/clock-warp.o ../libs/qemu/stubs/migr-blocker.o ../libs/qemu/stubs/mon-printf.o ../libs/qemu/stubs/vc-init.o ../libs/qemu/stubs/chr-baum-init.o ../libs/qemu/stubs/is-daemonized.o ../libs/qemu/stubs/dump.o ../libs/qemu/stubs/mon-set-error.o ../libs/qemu/stubs/runstate-check.o ../libs/qemu/stubs/reset.o ../libs/qemu/stubs/cpu-get-clock.o ../libs/qemu/stubs/mon-is-qmp.o ../libs/qemu/stubs/kvm.o ../libs/qemu/stubs/pci-drive-hot-add.o ../libs/qemu/stubs/fdset-get-fd.o ../libs/qemu/stubs/get-fd.o ../libs/qemu/stubs/fdset-find-fd.o ../libs/qemu/stubs/vm-stop.o ../libs/qemu/stubs/fdset-remove-fd.o ../libs/qemu/stubs/monitor-init.o ../libs/qemu/stubs/fdset-add-fd.o ../libs/qemu/stubs/qtest.o ../libs/qemu/stubs/uuid.o ../libs/qemu/stubs/gdbstub.o ../libs/qemu/stubs/cpu-get-icount.o ../libs/qemu/stubs/chr-msmouse.o ../libs/qemu/stubs/iothread-lock.o ../libs/qemu/stubs/machine-init-done.o ../libs/qemu/stubs/get-vm-name.o ../libs/qemu/stubs/vmstate.o ../libs/qemu/stubs/cpus.o ../libs/qemu/stubs/get-next-serial.o ../libs/qemu/stubs/sysbus.o ../libs/qemu/x86_64-linux-user/linux-user/main.o ../libs/qemu/block/block-backend.o ../libs/qemu/block/accounting.o ../libs/qemu/util/id.o ../libs/qemu/qapi/qapi-util.o `llvm-config --libs --cflags --ldflags` -lz3 -ldl -lncurses  -g -fsanitize=address,leak -fno-omit-frame-pointer -fno-common -DNEED_CPU_H -DCONFIG_USER_ONLY -pthread -W -Wall -Wextra

You can now test the program:

    ./testme 48c7c034120000c3
