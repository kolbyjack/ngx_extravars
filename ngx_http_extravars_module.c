
/*
 * Copyright (C) Jonathan Kolb
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#define NGX_EXTRAVAR_TIME_NOW       0
#define NGX_EXTRAVAR_TIME_ELAPSED   1
#define NGX_EXTRAVAR_TIME_REQUEST   2

#define NGX_EXTRAVAR_WSGI_SCRIPT_NAME   0
#define NGX_EXTRAVAR_WSGI_PATH_INFO     1

#define NGX_EXTRAVAR_STUB_STAT_ACCEPTED     0
#define NGX_EXTRAVAR_STUB_STAT_ACTIVE       1
#define NGX_EXTRAVAR_STUB_STAT_HANDLED      2
#define NGX_EXTRAVAR_STUB_STAT_READING      3
#define NGX_EXTRAVAR_STUB_STAT_REQUESTS     4
#define NGX_EXTRAVAR_STUB_STAT_WAITING      5
#define NGX_EXTRAVAR_STUB_STAT_WRITING      6

#define NGX_EXTRAVAR_REDIRECT_COUNT         0
#define NGX_EXTRAVAR_SUBREQUEST_COUNT       1
#define NGX_EXTRAVAR_PROCESS_SLOT           2
#define NGX_EXTRAVAR_CONNECTION_REQUESTS    3
#define NGX_EXTRAVAR_RANDOM                 4

#define NGX_EXTRAVARS_PRE_1_3_2 ((nginx_version < 1002002) || ((nginx_version >= 1003000) && (nginx_version < 1003002)))
#define NGX_EXTRAVARS_PRE_1_3_8 (nginx_version < 1003008)
#define NGX_EXTRAVARS_PRE_1_3_9 (nginx_version < 1003009)
#define NGX_EXTRAVARS_PRE_1_3_12 (nginx_version < 1003012)

static ngx_int_t ngx_http_extravars_add_variables(ngx_conf_t *cf);

static ngx_int_t ngx_extra_var_aliased_uri(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_location(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_time_msec(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_uint(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_ext(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_original_uri(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_request_version(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_wsgi(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

#if (NGX_HTTP_CACHE)
static ngx_int_t ngx_extra_var_cache_key(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_cache_file(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_cache_age(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
#endif

#if (NGX_STAT_STUB)
static ngx_int_t ngx_extra_var_stub_stat(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
#endif

#if (NGX_EXTRAVARS_PRE_1_3_2)
static ngx_int_t ngx_extra_var_status(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
#endif

#if (NGX_EXTRAVARS_PRE_1_3_8)
static ngx_int_t ngx_extra_var_bytes_sent(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_connection(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
#endif

#if (NGX_EXTRAVARS_PRE_1_3_12)
static ngx_int_t ngx_extra_var_pipe(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_time_local(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_iso8601(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_extra_var_request_length(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
#endif


static ngx_http_module_t  ngx_http_extravars_module_ctx = {
    ngx_http_extravars_add_variables,      /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_extravars_module = {
    NGX_MODULE_V1,
    &ngx_http_extravars_module_ctx,        /* module context */
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_variable_t  ngx_http_extra_variables[] = {

    { ngx_string("aliased_uri"), NULL, ngx_extra_var_aliased_uri, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("ext"), NULL, ngx_extra_var_ext, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("location"), NULL, ngx_extra_var_location, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("original_uri"), NULL, ngx_extra_var_original_uri, 0, 0, 0 },

    { ngx_string("process_slot"), NULL, ngx_extra_var_uint,
        NGX_EXTRAVAR_PROCESS_SLOT, 0, 0 },

    { ngx_string("random"), NULL, ngx_extra_var_uint,
        NGX_EXTRAVAR_RANDOM, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("redirect_count"), NULL, ngx_extra_var_uint,
        NGX_EXTRAVAR_REDIRECT_COUNT, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("request_received"), NULL, ngx_extra_var_time_msec,
        NGX_EXTRAVAR_TIME_REQUEST, 0, 0 },

    { ngx_string("request_version"), NULL, ngx_extra_var_request_version,
        0, 0, 0 },

    { ngx_string("subrequest_count"), NULL, ngx_extra_var_uint,
        NGX_EXTRAVAR_SUBREQUEST_COUNT, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("wsgi_script_name"), NULL, ngx_extra_var_wsgi,
        NGX_EXTRAVAR_WSGI_SCRIPT_NAME, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("wsgi_path_info"), NULL, ngx_extra_var_wsgi,
        NGX_EXTRAVAR_WSGI_PATH_INFO, NGX_HTTP_VAR_NOCACHEABLE, 0 },

#if (NGX_HTTP_CACHE)
    { ngx_string("cache_age"), NULL, ngx_extra_var_cache_age, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("cache_file"), NULL, ngx_extra_var_cache_file, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("cache_key"), NULL, ngx_extra_var_cache_key, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },
#endif

#if (NGX_STAT_STUB)
    { ngx_string("stub_stat_accepted"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_ACCEPTED, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("stub_stat_active"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_ACTIVE, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("stub_stat_handled"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_HANDLED, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("stub_stat_reading"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_READING, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("stub_stat_requests"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_REQUESTS, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("stub_stat_waiting"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_WAITING, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("stub_stat_writing"), NULL, ngx_extra_var_stub_stat,
        NGX_EXTRAVAR_STUB_STAT_WRITING, NGX_HTTP_VAR_NOCACHEABLE, 0 },
#endif

#if (NGX_EXTRAVARS_PRE_1_3_2)
    { ngx_string("status"), NULL, ngx_extra_var_status, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },
#endif

#if (NGX_EXTRAVARS_PRE_1_3_8)
    { ngx_string("bytes_sent"), NULL, ngx_extra_var_bytes_sent, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("connection"), NULL, ngx_extra_var_connection, 0, 0, 0 },

    { ngx_string("connection_requests"), NULL, ngx_extra_var_uint,
        NGX_EXTRAVAR_CONNECTION_REQUESTS, 0, 0 },
#endif

#if (NGX_EXTRAVARS_PRE_1_3_9)
    { ngx_string("msec"), NULL, ngx_extra_var_time_msec,
        NGX_EXTRAVAR_TIME_NOW, NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("request_time"), NULL, ngx_extra_var_time_msec,
        NGX_EXTRAVAR_TIME_ELAPSED, NGX_HTTP_VAR_NOCACHEABLE, 0 },
#endif

#if (NGX_EXTRAVARS_PRE_1_3_12)
    { ngx_string("pipe"), NULL, ngx_extra_var_pipe, 0, 0, 0 },

    { ngx_string("request_length"), NULL, ngx_extra_var_request_length,
        0, 0, 0 },

    { ngx_string("time_iso8601"), NULL, ngx_extra_var_iso8601, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },

    { ngx_string("time_local"), NULL, ngx_extra_var_time_local, 0,
        NGX_HTTP_VAR_NOCACHEABLE, 0 },
#endif

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


static ngx_int_t
ngx_http_extravars_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_extra_variables; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_aliased_uri(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_core_loc_conf_t   *clcf;
    size_t                      alias;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
    alias =
#if (NGX_PCRE)
        clcf->regex ? 0 :
#endif
        clcf->name.len;

    if (r->uri.len >= alias
        && ngx_strncmp(r->uri.data, clcf->name.data, alias) == 0)
    {
        v->data = r->uri.data + alias;
        v->len = r->uri.len - alias;
    } else {
        v->data = r->uri.data;
        v->len = r->uri.len;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_ext(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->len = r->exten.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = r->exten.data;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_location(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    v->len = clcf->name.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = clcf->name.data;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_original_uri(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_str_t    uri, args;
    u_char      *src, *dst, *p;
    ngx_uint_t   flags;

    uri = r->unparsed_uri;
    ngx_str_null(&args);
    flags = 0;

    if (NGX_OK != ngx_http_parse_unsafe_uri(r, &uri, &args, &flags)) {
        v->not_found = 1;
        return NGX_OK;
    }

    p = ngx_pnalloc(r->pool, uri.len);
    if (p == NULL) {
        return NGX_ERROR;
    }

    src = uri.data;
    dst = p;

    ngx_unescape_uri(&dst, &src, uri.len, 0);

    v->len = dst - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_wsgi(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_core_loc_conf_t   *clcf;
    ngx_uint_t                  script_name_len;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    script_name_len =
#if (NGX_PCRE)
        clcf->regex ? 0 :
#endif
        clcf->name.len;

    if (script_name_len > 0
        && '/' == clcf->name.data[script_name_len - 1])
    {
        --script_name_len;
    }

    if (ngx_strncmp(r->uri.data, clcf->name.data, script_name_len) != 0) {
        script_name_len = 0;
    }

    if (NGX_EXTRAVAR_WSGI_SCRIPT_NAME == data) {
        v->data = r->uri.data;
        v->len = script_name_len;
    } else {
        v->data = r->uri.data + script_name_len;
        v->len = r->uri.len - script_name_len;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_time_msec(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char         *p;
    ngx_time_t     *tp, t;
    ngx_msec_int_t  ms;

    p = ngx_pnalloc(r->pool, NGX_TIME_T_LEN + 4);
    if (p == NULL) {
        return NGX_ERROR;
    }

    switch (data) {
    case NGX_EXTRAVAR_TIME_NOW :
        tp = ngx_timeofday();
        break;

    case NGX_EXTRAVAR_TIME_ELAPSED :
        tp = ngx_timeofday();

        ms = (ngx_msec_int_t) ((tp->sec - r->start_sec) * 1000
                 + (tp->msec - r->start_msec));
        ms = ngx_max(ms, 0);

        tp = &t;
        tp->sec = ms / 1000;
        tp->msec = ms % 1000;
        break;

    case NGX_EXTRAVAR_TIME_REQUEST :
        tp = &t;
        tp->sec = r->start_sec;
        tp->msec = r->start_msec;
        break;

    default :
        v->not_found = 1;
        return NGX_OK;
    }

    v->len = ngx_sprintf(p, "%T.%03M", tp->sec, tp->msec) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_uint(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_uint_t  value;
    u_char     *p;

    p = ngx_pnalloc(r->pool, NGX_INT_T_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    switch (data) {
    case NGX_EXTRAVAR_REDIRECT_COUNT:
        value = NGX_HTTP_MAX_URI_CHANGES + 1 - r->uri_changes;
        break;

    case NGX_EXTRAVAR_SUBREQUEST_COUNT:
        value = NGX_HTTP_MAX_SUBREQUESTS + 1 - r->subrequests;
        break;

    case NGX_EXTRAVAR_PROCESS_SLOT:
        value = ngx_process_slot;
        break;

    case NGX_EXTRAVAR_CONNECTION_REQUESTS:
        value = r->connection->requests;
        break;

    case NGX_EXTRAVAR_RANDOM:
        value = ngx_random();
        break;

    default:
        v->not_found = 1;
        return NGX_OK;
    }

    v->len = ngx_sprintf(p, "%ui", value) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_request_version(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char  *p;

    p = ngx_pnalloc(r->pool, NGX_INT_T_LEN * 2 + 1);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = ngx_sprintf(p, "%ui.%ui",
        r->http_major, r->http_minor) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


#if (NGX_HTTP_CACHE)
static ngx_int_t
ngx_extra_var_cache_key(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char            *p;
    ngx_http_cache_t  *c;

    c = r->cache;
    if (c == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    p = ngx_pnalloc(r->pool, 2 * NGX_HTTP_CACHE_KEY_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    ngx_hex_dump(p, c->key, NGX_HTTP_CACHE_KEY_LEN);

    v->len = 2 * NGX_HTTP_CACHE_KEY_LEN;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_cache_file(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_cache_t  *c;

    c = r->cache;
    if (c == NULL || 0 == c->file.name.len) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->len = c->file.name.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = c->file.name.data;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_cache_age(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    time_t age = 0;
    u_char *p;

    if (r->upstream == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    switch (r->upstream->cache_status) {
    case NGX_HTTP_CACHE_EXPIRED:
    case NGX_HTTP_CACHE_UPDATING:
        p = (u_char *) "0";
        v->len = 1;
        break;

    case NGX_HTTP_CACHE_STALE:
    case NGX_HTTP_CACHE_HIT:
        p = ngx_pnalloc(r->pool, NGX_TIME_T_LEN);
        if (p == NULL) {
            return NGX_ERROR;
        }

        age = ngx_time() - r->cache->date;
        v->len = ngx_sprintf(p, "%T", age) - p;
        break;

    default:
        v->not_found = 1;
        return NGX_OK;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}
#endif


#if (NGX_STAT_STUB)
static ngx_int_t
ngx_extra_var_stub_stat(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_atomic_t    stat;
    u_char         *p;

    switch (data) {
    case NGX_EXTRAVAR_STUB_STAT_ACCEPTED:
        stat = *ngx_stat_accepted;
        break;

    case NGX_EXTRAVAR_STUB_STAT_ACTIVE:
        stat = *ngx_stat_active;
        break;

    case NGX_EXTRAVAR_STUB_STAT_HANDLED:
        stat = *ngx_stat_handled;
        break;

    case NGX_EXTRAVAR_STUB_STAT_READING:
        stat = *ngx_stat_reading;
        break;

    case NGX_EXTRAVAR_STUB_STAT_REQUESTS:
        stat = *ngx_stat_requests;
        break;

    case NGX_EXTRAVAR_STUB_STAT_WAITING:
        stat = *ngx_stat_active - (*ngx_stat_reading + *ngx_stat_writing);
        break;

    case NGX_EXTRAVAR_STUB_STAT_WRITING:
        stat = *ngx_stat_writing;
        break;

    default:
        v->not_found = 1;
        return NGX_OK;
    }

    p = ngx_pnalloc(r->pool, NGX_ATOMIC_T_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = ngx_sprintf(p, "%uA", stat) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}
#endif


#if (NGX_EXTRAVARS_PRE_1_3_2)
static ngx_int_t
ngx_extra_var_status(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char  *p;
    ngx_uint_t  status;

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    if (r->err_status) {
        status = r->err_status;

    } else if (r->headers_out.status) {
        status = r->headers_out.status;

    } else if (r->http_version == NGX_HTTP_VERSION_9) {
        v->len = 3;
        v->data = (u_char *)"009";
        return NGX_OK;

    } else {
        status = 0;
    }

    p = ngx_pnalloc(r->pool, NGX_INT_T_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = ngx_sprintf(p, "%03ui", status) - p;
    v->data = p;

    return NGX_OK;
}
#endif


#if (NGX_EXTRAVARS_PRE_1_3_8)
static ngx_int_t
ngx_extra_var_bytes_sent(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char  *p;

    p = ngx_pnalloc(r->pool, NGX_OFF_T_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = ngx_sprintf(p, "%O", r->connection->sent) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_connection(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char  *p;

    p = ngx_pnalloc(r->pool, NGX_ATOMIC_T_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = ngx_sprintf(p, "%uA", r->connection->number) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}
#endif


#if (NGX_EXTRAVARS_PRE_1_3_12)
static ngx_int_t
ngx_extra_var_pipe(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->len = 1;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = (u_char *) ((r->pipeline) ? "p" : ".");

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_time_local(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->len = ngx_cached_http_log_time.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = ngx_cached_http_log_time.data;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_iso8601(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->len = ngx_cached_http_log_iso8601.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = ngx_cached_http_log_iso8601.data;

    return NGX_OK;
}


static ngx_int_t
ngx_extra_var_request_length(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char  *p;

    p = ngx_pnalloc(r->pool, NGX_OFF_T_LEN);
    if (p == NULL) {
        return NGX_ERROR;
    }

    v->len = ngx_sprintf(p, "%O", r->request_length) - p;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = p;

    return NGX_OK;
}
#endif

