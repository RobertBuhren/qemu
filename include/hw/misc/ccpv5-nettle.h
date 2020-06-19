/*
 * AMD PSP emulation
 *
 * Copyright (C) 2020 Robert Buhren <robert@robertbuhren.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CCP5 gnu nettle bindings
 */

#ifndef AMD_CCP_V5_NETTLE_H
#define AMD_CCP_V5_NETTLE_H
#include <nettle/sha2.h>
#include <glib.h>

/* TODO: Should this be in a dedicated c file? */

/* The CCP holds it's SHA context in the LSBs. However, to make use of the SHA
 * functionality of libraries such as nettle, we need storage for the library
 * specific sha context. The "CcpV5ShaCtx" represents such a context. It will
 * be allocated on demand and freed when the hash is computed.
 * The contexts here are "nettle" specific.
 */
typedef union {
    struct sha256_ctx *ctx_256;
    struct sha384_ctx *ctx_384;
    void* raw;
} CcpV5ShaCtx;

static inline void ccp_init_sha256_ctx(CcpV5ShaCtx *ctx) {
    if(ctx != NULL) {
        ctx->ctx_256 = (struct sha256_ctx*)g_malloc(sizeof(struct sha256_ctx));
        nettle_sha256_init(ctx->ctx_256);
    }
}

/* static void ccp_init_sha384_ctx(CcpV5ShaCtx *ctx) { */
/*     if(ctx != NULL) { */
/*         ctx->ctx_384 = (struct sha384_ctx*)g_malloc(sizeof(struct sha384_ctx)); */
/*         nettle_sha384_init(ctx->ctx_384); */
/*     } */
/* } */

static inline void ccp_update_sha256(CcpV5ShaCtx *ctx, uint32_t len, void *src) {
    nettle_sha256_update(ctx->ctx_256, len, src);
}

static inline void ccp_clear_sha_ctx(CcpV5ShaCtx *ctx) {
    if(ctx != NULL) {
        g_free(ctx->raw);
        ctx->raw = NULL;
    }
}

static inline void ccp_digest_sha256(CcpV5ShaCtx *ctx, uint8_t* lsb_ctx) {
    nettle_sha256_digest(ctx->ctx_256, SHA256_DIGEST_SIZE, lsb_ctx);
    ccp_clear_sha_ctx(ctx);
}

#endif
