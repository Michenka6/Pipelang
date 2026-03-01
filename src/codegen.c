#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/ir.h"

static int sv_equals(Nob_String_View sv, const char *cstr)
{
    size_t n = strlen(cstr);
    return sv.count == n && memcmp(sv.data, cstr, n) == 0;
}

static void print_sv_as_ident(Nob_String_View sv)
{
    for (size_t i = 0; i < sv.count; i++) {
        char c = sv.data[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_') {
            putchar(c);
        } else {
            putchar('_');
        }
    }
}

static bool idx_valid(const IR_Nodes *ir, size_t idx)
{
    return idx != (size_t)-1 && idx < ir->count;
}

static bool sv_empty(Nob_String_View sv)
{
    return sv.data == NULL || sv.count == 0;
}

static void collect_concat_leaves(const IR_Nodes *ir, size_t idx, size_t *buf, size_t *count, size_t cap)
{
    if (!idx_valid(ir, idx)) return;
    IR_Node n = ir->items[idx];
    if (n.kind == IR_Node_Kind_CONCAT) {
        collect_concat_leaves(ir, n.lhs, buf, count, cap);
        collect_concat_leaves(ir, n.rhs, buf, count, cap);
    } else {
        if (*count < cap) buf[(*count)++] = idx;
    }
}

void generate_c_from_ir(const IR_Nodes *ir, size_t root)
{
    assert(ir);
    assert(root < ir->count);

    printf("// Generated C (stage backend)\n");
    printf("#include <stdint.h>\n");
    printf("#include <stddef.h>\n");
    printf("#include <stdlib.h>\n");
    printf("#include <string.h>\n");
    printf("#include <stdio.h>\n");
    printf("#include <pthread.h>\n\n");

    printf("typedef struct Stream {\n");
    printf("    void  *data;\n");
    printf("    size_t count;\n");
    printf("    size_t elem_size;\n");
    printf("} Stream;\n\n");

    printf("static Stream stream_alloc(size_t elem_size, size_t count) {\n");
    printf("    Stream s = {0};\n");
    printf("    s.count = count;\n");
    printf("    s.elem_size = elem_size;\n");
    printf("    s.data = malloc(elem_size * count);\n");
    printf("    if (!s.data) { fprintf(stderr, \"OOM\\n\"); exit(1); }\n");
    printf("    return s;\n");
    printf("}\n\n");

    printf("static void stream_free(Stream *s) {\n");
    printf("    free(s->data);\n");
    printf("    s->data = NULL;\n");
    printf("    s->count = 0;\n");
    printf("    s->elem_size = 0;\n");
    printf("}\n\n");

    printf("typedef struct { void (*fn)(Stream*,Stream*); Stream *in; Stream *out; } Task;\n");
    printf("static void* run_task(void* p){ Task* t=(Task*)p; t->fn(t->in,t->out); return NULL; }\n\n");

    printf("// ---- primitive implementations (generated) ----\n\n");
    printf("typedef struct { uint8_t r,g,b; } RGB8;\n\n");

    printf("static void stage_impl_rgb(Stream *in, Stream *out) {\n");
    printf("    // Expect RGB8 stream.\n");
    printf("    *out = *in;\n");
    printf("}\n\n");

    printf("static void stage_impl_rgb_r(Stream *in, Stream *out) {\n");
    printf("    if (in->elem_size != sizeof(RGB8)) { fprintf(stderr, \"rgb/r: bad input elem_size\\n\"); exit(1); }\n");
    printf("    Stream o = stream_alloc(sizeof(uint8_t), in->count);\n");
    printf("    RGB8 *p = (RGB8*)in->data;\n");
    printf("    uint8_t *q = (uint8_t*)o.data;\n");
    printf("    for (size_t i=0;i<in->count;i++) q[i] = p[i].r;\n");
    printf("    *out = o;\n");
    printf("}\n\n");

    printf("static void stage_impl_rgb_g(Stream *in, Stream *out) {\n");
    printf("    if (in->elem_size != sizeof(RGB8)) { fprintf(stderr, \"rgb/g: bad input elem_size\\n\"); exit(1); }\n");
    printf("    Stream o = stream_alloc(sizeof(uint8_t), in->count);\n");
    printf("    RGB8 *p = (RGB8*)in->data;\n");
    printf("    uint8_t *q = (uint8_t*)o.data;\n");
    printf("    for (size_t i=0;i<in->count;i++) q[i] = p[i].g;\n");
    printf("    *out = o;\n");
    printf("}\n\n");

    printf("static void stage_impl_rgb_b(Stream *in, Stream *out) {\n");
    printf("    if (in->elem_size != sizeof(RGB8)) { fprintf(stderr, \"rgb/b: bad input elem_size\\n\"); exit(1); }\n");
    printf("    Stream o = stream_alloc(sizeof(uint8_t), in->count);\n");
    printf("    RGB8 *p = (RGB8*)in->data;\n");
    printf("    uint8_t *q = (uint8_t*)o.data;\n");
    printf("    for (size_t i=0;i<in->count;i++) q[i] = p[i].b;\n");
    printf("    *out = o;\n");
    printf("}\n\n");

    printf("static void stage_impl_u8_to_f32(Stream *in, Stream *out) {\n");
    printf("    if (in->elem_size != sizeof(uint8_t)) { fprintf(stderr, \"u8/to_f32: bad input elem_size\\n\"); exit(1); }\n");
    printf("    Stream o = stream_alloc(sizeof(float), in->count);\n");
    printf("    uint8_t *p = (uint8_t*)in->data;\n");
    printf("    float *q = (float*)o.data;\n");
    printf("    for (size_t i=0;i<in->count;i++) q[i] = (float)p[i];\n");
    printf("    *out = o;\n");
    printf("}\n\n");

    printf("static void stage_impl_f32_to_u8(Stream *in, Stream *out) {\n");
    printf("    if (in->elem_size != sizeof(float)) { fprintf(stderr, \"f32/to_u8: bad input elem_size\\n\"); exit(1); }\n");
    printf("    Stream o = stream_alloc(sizeof(uint8_t), in->count);\n");
    printf("    float *p = (float*)in->data;\n");
    printf("    uint8_t *q = (uint8_t*)o.data;\n");
    printf("    for (size_t i=0;i<in->count;i++) {\n");
    printf("        float x = p[i];\n");
    printf("        if (x < 0.0f) x = 0.0f;\n");
    printf("        if (x > 255.0f) x = 255.0f;\n");
    printf("        q[i] = (uint8_t)(x + 0.5f);\n");
    printf("    }\n");
    printf("    *out = o;\n");
    printf("}\n\n");

    printf("// ---- unknown FUN fallbacks (pass-through) ----\n");
    for (size_t i = 0; i < ir->count; i++) {
        IR_Node n = ir->items[i];
        if (n.kind != IR_Node_Kind_FUN) continue;
        if (sv_empty(n.name)) continue;

        if (sv_equals(n.name, "rgb") ||
            sv_equals(n.name, "rgb/r") ||
            sv_equals(n.name, "rgb/g") ||
            sv_equals(n.name, "rgb/b") ||
            sv_equals(n.name, "u8/to_f32") ||
            sv_equals(n.name, "f32/to_u8")) {
            continue;
        }

        printf("static void stage_impl_");
        print_sv_as_ident(n.name);
        printf("(Stream *in, Stream *out) {\n");
        printf("    // TODO: implement ");
        fwrite(n.name.data, 1, n.name.count, stdout);
        printf("\n");
        printf("    *out = *in;\n");
        printf("}\n\n");
    }

    printf("// ---- generated stages ----\n");
    for (size_t i = 0; i < ir->count; i++) {
        printf("static void stage_%zu(Stream*,Stream*);\n", i);
    }
    printf("\n");

    for (size_t i = 0; i < ir->count; i++) {
        IR_Node n = ir->items[i];

        printf("static void stage_%zu(Stream* in, Stream* out)\n{\n", i);

        switch (n.kind) {

        case IR_Node_Kind_FUN:
            if (sv_empty(n.name)) {
                printf("    // FUN: <empty> treated as NOP\n");
                printf("    *out = *in;\n");
            } else {
                printf("    // FUN\n");
                printf("    stage_impl_");
                print_sv_as_ident(n.name);
                printf("(in,out);\n");
            }
            break;

        case IR_Node_Kind_INT:
            printf("    // INT literal -> scalar float stream\n");
            printf("    *out = stream_alloc(sizeof(float), 1);\n");
            printf("    ((float*)out->data)[0] = (float)%lld;\n", (long long)n.iv);
            break;

        case IR_Node_Kind_FLOAT:
            printf("    // FLOAT literal -> scalar float stream\n");
            printf("    *out = stream_alloc(sizeof(float), 1);\n");
            printf("    ((float*)out->data)[0] = (float)%0.9g;\n", n.fv);
            break;

        case IR_Node_Kind_CONCAT:
            printf("    // CONCAT: lhs -> rhs\n");
            if (!idx_valid(ir, n.lhs) || !idx_valid(ir, n.rhs)) {
                printf("    // ERROR: invalid CONCAT child index\n");
                printf("    *out = *in;\n");
                break;
            }
            printf("    Stream mid={0};\n");
            printf("    stage_%zu(in, &mid);\n", n.lhs);
            printf("    stage_%zu(&mid, out);\n", n.rhs);
            printf("    // mid may alias in (pass-through)\n");
            printf("    if (mid.data != in->data) stream_free(&mid);\n");
            break;

        case IR_Node_Kind_UNARY:
        {
            size_t kid = (idx_valid(ir, n.rhs) ? n.rhs : (idx_valid(ir, n.lhs) ? n.lhs : (size_t)-1));

            printf("    // UNARY: elementwise with scalar produced by child\n");
            if (kid == (size_t)-1) {
                printf("    // ERROR: UNARY has no scalar child\n");
                printf("    *out = *in;\n");
                break;
            }

            printf("    if (in->elem_size != sizeof(float)) { fprintf(stderr, \"UNARY: expected float input\\n\"); exit(1); }\n");
            printf("    Stream kstream={0};\n");
            printf("    stage_%zu(in, &kstream);\n", kid);
            printf("    if (kstream.elem_size != sizeof(float) || kstream.count < 1) { fprintf(stderr, \"UNARY: bad scalar\\n\"); exit(1); }\n");
            printf("    float k = ((float*)kstream.data)[0];\n");

            printf("    *out = stream_alloc(sizeof(float), in->count);\n");
            printf("    float *src = (float*)in->data;\n");
            printf("    float *dst = (float*)out->data;\n");
            printf("    for (size_t j=0; j<in->count; j++) {\n");
            printf("        float x = src[j];\n");

            switch (n.op) {
                case Op_TIMES:  printf("        dst[j] = x * k;\n"); break;
                case Op_POW:    printf("        dst[j] = x ^ k;\n"); break;
                case Op_PLUS:   printf("        dst[j] = x + k;\n"); break;
                case Op_MINUS:  printf("        dst[j] = x - k;\n"); break;
                case Op_DIVIDE: printf("        dst[j] = x / k;\n"); break;
                case Op_MIN:    printf("        dst[j] = (x < k) ? x : k;\n"); break;
                case Op_MAX:    printf("        dst[j] = (x > k) ? x : k;\n"); break;
                case Op_GR:     printf("        dst[j] = (x > k) ? 1.0f : 0.0f;\n"); break;
                case Op_LEQ:    printf("        dst[j] = (x <= k) ? 1.0f : 0.0f;\n"); break;
                default:        printf("        dst[j] = x; /* unknown op */\n"); break;
            }

            printf("    }\n");
            printf("    stream_free(&kstream);\n");
        } break;

        case IR_Node_Kind_UNFOLD:
        case IR_Node_Kind_FOLD:
        {
            printf("    // %s\n", (n.kind == IR_Node_Kind_UNFOLD ? "UNFOLD" : "FOLD"));

            if (sv_empty(n.name) || !sv_equals(n.name, "sum")) {
                printf("    // Unsupported reducer: fallback to lhs\n");
                if (idx_valid(ir, n.lhs)) printf("    stage_%zu(in, out);\n", n.lhs);
                else printf("    *out = *in;\n");
                break;
            }

            if (!idx_valid(ir, n.lhs)) {
                printf("    // ERROR: UNFOLD lhs invalid\n");
                printf("    *out = *in;\n");
                break;
            }

            size_t leaves[256];
            size_t leaf_count = 0;
            collect_concat_leaves(ir, n.lhs, leaves, &leaf_count, 256);

            size_t branches[256];
            size_t bcount = 0;
            for (size_t t = 0; t < leaf_count; t++) {
                size_t bi = leaves[t];
                IR_Node bn = ir->items[bi];
                if (bn.kind == IR_Node_Kind_FUN && sv_empty(bn.name)) continue;
                branches[bcount++] = bi;
            }

            printf("    // sum: run %zu branches on same input and reduce elementwise\n", bcount);
            printf("    Stream outs[%zu]; memset(outs, 0, sizeof(outs));\n", bcount);
            printf("    pthread_t th[%zu];\n", bcount);
            printf("    Task tasks[%zu];\n", bcount);

            for (size_t b = 0; b < bcount; b++) {
                printf("    tasks[%zu].fn = stage_%zu;\n", b, branches[b]);
                printf("    tasks[%zu].in = in;\n", b);
                printf("    tasks[%zu].out = &outs[%zu];\n", b, b);
                printf("    pthread_create(&th[%zu], NULL, run_task, &tasks[%zu]);\n", b, b);
            }
            for (size_t b = 0; b < bcount; b++) {
                printf("    pthread_join(th[%zu], NULL);\n", b);
            }

            printf("    // all branches must produce float streams\n");
            printf("    *out = stream_alloc(sizeof(float), in->count);\n");
            printf("    float *dst = (float*)out->data;\n");
            printf("    for (size_t j=0; j<in->count; j++) {\n");
            printf("        float acc = 0.0f;\n");
            for (size_t b = 0; b < bcount; b++) {
                printf("        if (outs[%zu].elem_size != sizeof(float)) { fprintf(stderr, \"sum: branch not float\\n\"); exit(1); }\n", b);
                printf("        acc += ((float*)outs[%zu].data)[j];\n", b);
            }
            printf("        dst[j] = acc;\n");
            printf("    }\n");

            for (size_t b = 0; b < bcount; b++) {
                printf("    stream_free(&outs[%zu]);\n", b);
            }
        } break;

        case IR_Node_Kind_FORK:
            printf("    // FORK not used in this minimal generator; fallback\n");
            if (idx_valid(ir, n.lhs)) {
                printf("    stage_%zu(in, out);\n", n.lhs);
            } else {
                printf("    *out = *in;\n");
            }
            break;

        default:
            printf("    // unsupported node kind\n");
            printf("    *out = *in;\n");
            break;
        }

        printf("}\n\n");
    }

    printf("void run_pipeline(Stream* in, Stream* out)\n{\n");
    printf("    stage_%zu(in, out);\n", root);
    printf("}\n\n");

    printf("#ifdef PIPE_DEMO_MAIN\n");
    printf("int main(void)\n{\n");
    printf("    // Demo input: 4 RGB pixels\n");
    printf("    Stream in = stream_alloc(sizeof(RGB8), 4);\n");
    printf("    RGB8 *p = (RGB8*)in.data;\n");
    printf("    p[0] = (RGB8){255,0,0};\n");
    printf("    p[1] = (RGB8){0,255,0};\n");
    printf("    p[2] = (RGB8){0,0,255};\n");
    printf("    p[3] = (RGB8){255,255,255};\n");
    printf("\n");
    printf("    Stream out = {0};\n");
    printf("    run_pipeline(&in, &out);\n");
    printf("\n");
    printf("    if (out.elem_size == sizeof(uint8_t)) {\n");
    printf("        uint8_t *g = (uint8_t*)out.data;\n");
    printf("        for (size_t i=0;i<out.count;i++) printf(\"%%u\\n\", (unsigned)g[i]);\n");
    printf("    } else {\n");
    printf("        fprintf(stderr, \"unexpected output elem_size=%%zu\\n\", out.elem_size);\n");
    printf("    }\n");
    printf("\n");
    printf("    stream_free(&in);\n");
    printf("    if (out.data && out.data != in.data) stream_free(&out);\n");
    printf("    return 0;\n");
    printf("}\n");
    printf("#endif\n");
}
