/**
 * Filename: management-internal.h
 *
 * Description: Management internal data-types and function proto-types.
 *
 * Do NOT modify or remove this copyright and confidentiality notice!
 * Copyright (c) 2019, Seagate Technology, LLC.
 * The code contained herein is CONFIDENTIAL to Seagate Technology, LLC.
 * Portions are also trade secret. Any use, duplication, derivation, distribution
 * or disclosure of this code, for any reason, not expressly authorized is
 * prohibited. All other rights are expressly reserved by Seagate Technology, LLC.
 * 
 * Author: Yogesh Lahane <yogesh.lahane@seagate.com>
 *
 */

#ifndef _MANAGEMENT_INTERNAL_H_
#define _MANAGEMENT_INTERNAL_H_

#include <event.h> /* evbuf_t */
#include <event2/http.h> /* evhttp_uridecode */
#include <evhtp.h> /* evhtp_t, evhtp_request_t, evhtp_header_t */

/**
 * Controller api_new function pointer data types.
 */
typedef int (*controller_api_init_func)
	     (char *api_name,
	     struct controller *controller,
	     struct request *request,
	     struct controller_api **api);
typedef void (*controller_api_fini_func)(struct controller_api *fs_api);
/**
 * Controller - APIs.
 */
struct controller* controller_find_by_api_uri(struct server *server,
					       char *api_uri);
struct controller* controller_find_by_name(struct server *server, char *name);

/**
 * ######################################################################
 * #		Control-Server: CONTROLLER-API Data Type and APIs.	#
 * ######################################################################
 */
typedef int (*controller_api_action_func)(struct controller_api *api,
					  void *args);
/**
 * Controller-api - Data Type.
 */
struct controller_api {
	struct controller		*controller;	/* controller. */
	struct request			*request;	/* request. */
	char				*name;		/* api name. */
	int		 	  	 type;		/* api type. */
	int 				 action_next;	/* Next api action */
	controller_api_action_func	*action_table; /* api action table. */
	void				*priv;		/* api private. */
};

struct controller_api_table {
	char	*name;		/* api name. */
	char	*method;	/* api method. */
	int	 id;		/* api id. */
};

/**
 * ######################################################################
 * #		Control-Server: REQUEST data type and APIs.		#
 * ######################################################################
 */
enum request_state {
	RUNNING = 0,
	PAUSED,
	STOPPED,
	WAITING,
	ERROR,
};

enum http_method {
	HEAD = htp_method_HEAD,
#define STR_HTTP_METHOD_HEAD	"HEAD"
	GET = htp_method_GET,
#define STR_HTTP_METHOD_GET	"GET"
	PUT = htp_method_PUT,
#define STR_HTTP_METHOD_PUT	"PUT"
	DELETE = htp_method_DELETE,
#define STR_HTTP_METHOD_DELETE	"DELETE"
	POST = htp_method_POST,
#define STR_HTTP_METHOD_POST	"POST"
	UNKNOWN,
#define STR_HTTP_METHOD_UNKNOWN	"UNKNOWN"
};

enum http_protocol {
	HTTP_PROTO_10 = EVHTP_PROTO_10,
	HTTP_PROTO_11 = EVHTP_PROTO_11,
	HTTP_PROTO_UNKNOWN = EVHTP_PROTO_INVALID,
};

struct header {
	char *key;
	char *val;
};

typedef int (*request_read_cb_func)(struct controller_api *api);

struct request {
	evhtp_request_t		*evhtp_req;	/* evhtp request holder. */
	struct http		*http;		/* evhtp handlers. */

	struct server		*server;	/* Request server back link. */
	struct controller	*controller;	/* Request controller. */

	/* Request Info. */
	enum http_protocol	 http_proto;
	enum http_method	 http_method;
	char			*api_file;
	char			*api_uri;
	char			*api_path;	
	char			*api_method;
	char			*api_query;

	/* Request headers. */
	LIST_HEAD(in_headers,
		  header)	 in_headers;
	LIST_HEAD(out_headers,
		  header)	 out_headers;

	/* api handler. */
	struct controller_api   *api;		/* Controller api. */

	/* Request Data info. */
	evbuf_t			*in_buffer;
	int			 in_content_len;
	int			 in_read_len;
	int			 in_remaining_len;
	struct json_object	*in_json_req_obj;

	/* Response data info. */
	evbuf_t			*out_buffer;
	int			 out_content_len;
	struct json_object	*out_json_req_obj;
	int			 err_code;

	/* Request Operations handlers and call backs. */
	request_read_cb_func	 read_cb;

	/* General Info. */
	int			 is_client_disconnected;
	enum request_state	 state;	/* Request state. */
};

/**
 * Request instance APIs.
 */
int request_init(struct server *server,
		 evhtp_request_t *evhtp_req,
		 struct request **request);
void request_fini(struct request *req);

/**
 * Request APIs.
 */
struct controller* request_get_controller(struct request *request);
struct controller_api* request_get_api(struct request *request);
int request_validate_headers(struct request *request);
int request_accept_data(struct request *request);
void request_execute(struct controller_api *api);
void request_next_action(struct controller_api *api);
void request_send_responce(struct request *request, int code);
void request_set_out_header(struct request *request,
			    const char *key,
			    const char *value);

/**
 * HTTP APIs.
 */
int http_evhtp_init(evbase_t *ev_base,
		    struct server *server,
		    evhtp_t **evhtp);
void http_evhtp_fini(evhtp_t *ev_htp);

typedef void (*http_request_pause_func)(evhtp_request_t *request);
typedef void (*http_request_resume_func)(evhtp_request_t *request);
typedef evhtp_proto (*http_request_get_proto_func)(evhtp_request_t *request);

typedef evhtp_header_t* (*http_header_new_func)(const char *key,
						const char *val,
						char kalloc,
						char valloc);
typedef const char* (*http_header_find_func)(evhtp_headers_t *headers,
					     const char *key);
typedef void (*http_headers_add_header_func)(evhtp_headers_t *headers,
					     evhtp_header_t *header);

typedef const char* (*http_kv_find_func)(evhtp_kvs_t *kvs, const char *key);
typedef evhtp_kv_t* (*http_kvs_find_kv_func)(evhtp_kvs_t *kvs, const char *key);
typedef int (*http_kvs_for_each_func)(evhtp_kvs_t *kvs,
				      evhtp_kvs_iterator cb,
				      void *arg);

typedef void (*http_send_reply_func)(evhtp_request_t *request, evhtp_res code);
typedef void (*http_send_reply_start_func)(evhtp_request_t *request,
					   evhtp_res code);
typedef void (*http_send_reply_body_func)(evhtp_request_t *request, evbuf_t *buf);
typedef void (*http_send_reply_end_func)(evhtp_request_t *request);
typedef size_t (*http_evbuffer_get_length_func)(const struct evbuffer *buf);

struct http {
	evhtp_request_t			*evhtp_req;
	/* evhtp handlers. */
	http_request_pause_func		 request_pause;
	http_request_resume_func	 request_resume;
	http_request_get_proto_func	 request_get_proto;
	http_kvs_for_each_func		 kvs_for_each;
	http_header_find_func		 header_find;
	http_headers_add_header_func	 headers_add_header;
	http_header_new_func		 header_new;
	http_kv_find_func		 kv_find;
	http_kvs_find_kv_func		 kvs_find_kv;
	http_send_reply_func		 send_reply;
	http_send_reply_start_func	 send_reply_start;
	http_send_reply_body_func	 send_reply_body;
	http_send_reply_end_func	 send_reply_end;
	http_evbuffer_get_length_func	 evbuffer_get_length;
};

int http_init(evhtp_request_t *evhtp_req, struct http **http);
void http_fini(struct http *http);

#endif