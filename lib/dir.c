/* vim: set tabstop=8 shiftwidth=4 softtabstop=4 expandtab smarttab colorcolumn=80: */

#include "misc.h"
#include <jose/b64.h>
#include <jose/jwk.h>
#include <jose/jwe.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <string.h>

static const char *
suggest(const json_t *jwk)
{
    static const char *encs[] = {
        "A128CBC-HS256",
        "A192CBC-HS384",
        "A256CBC-HS512",
        "A128GCM",
        "A192GCM",
        "A256GCM",
        NULL
    };

    const char *alg = NULL;

    if (json_unpack((json_t *) jwk, "{s:s}", "alg", &alg) == -1)
        return NULL;

    for (size_t i = 0; encs[i]; i++) {
        if (strcmp(encs[i], alg) == 0)
            return "dir";
    }

    return NULL;
}

static bool
copy(json_t *to, const json_t *from)
{
    json_t *cpy = NULL;
    bool ret = false;

    cpy = json_deep_copy(from);
    if (!cpy)
        return false;

    ret = json_object_update(to, cpy) == 0;
    json_decref(cpy);
    return ret;
}

static bool
wrap(json_t *jwe, json_t *cek, const json_t *jwk, json_t *rcp,
     const char *alg)
{
    if (!json_object_get(cek, "k") && !copy(cek, jwk))
        return false;

    return json_object_set_new(rcp, "encrypted_key", json_string("")) == 0;
}

static bool
unwrap(const json_t *jwe, const json_t *jwk, const json_t *rcp,
       const char *alg, json_t *cek)
{
    return copy(cek, jwk);
}

static void __attribute__((constructor))
constructor(void)
{
    static const char *names[] = { "dir", NULL };

    static jose_jwe_wrapper_t aeskw_wrapper = {
        .algs = names,
        .suggest = suggest,
        .wrap = wrap,
        .unwrap = unwrap,
    };

    jose_jwe_register_wrapper(&aeskw_wrapper);
}
