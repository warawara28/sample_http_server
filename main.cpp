#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

class HttpServer {
public:
  HttpServer() :
    _base(nullptr), _httpd(nullptr), _address("0.0.0.0"), _port(3939)
  {
  }

  ~HttpServer(){}

  void start() {
    if((_base = event_base_new()) == nullptr)                 server_error("event_base_new()");
    if((_httpd = evhttp_new(_base)) == nullptr)               server_error("evhttp_new()");
    if (evhttp_bind_socket(_httpd, _address, _port) < 0)      server_error("evhttp_bind_socket()");

    //evhttp_set_cb(_httpd, "/userauth", HttpServer::userauthLauncher, this);
    evhttp_set_gencb(_httpd, requestHandler, this);
    std::cout << "http://" << _address << ":" << _port << " is accepted" << std::endl;
    event_base_dispatch(_base);

    evhttp_free(_httpd);
    event_base_free(_base);
  }

private:

  static void requestHandler(struct evhttp_request *r, void *) {
    if (r->type != EVHTTP_REQ_GET) {
        evhttp_send_error(r, HTTP_BADREQUEST, "Available GET only");
        return;
    }

    const char* uri = evhttp_request_get_uri(r);
    const struct evhttp_uri* http_uri = evhttp_uri_parse_with_flags(uri, EVHTTP_URI_NONCONFORMANT);
    const char* query = evhttp_uri_get_query(http_uri);

    std::cout << "uri  : " << uri << std::endl;
    std::cout << "query: " << query << std::endl;

    std::unordered_map<std::string, std::string> params = parseQuery(query);

    /*
    if (params.size() <= 0) {
      evhttp_send_error(r, HTTP_BADREQUEST, "no parameter");
      return;
    }
    */

    std::string dest("https://example.com/");
    dest += query;
    redirect(r, dest);
  }

  static std::unordered_map<std::string, std::string> parseQuery(const char* query){
    std::unordered_map<std::string, std::string> params;
    struct evkeyvalq headers;
    evhttp_parse_query_str(query, &headers);

    struct evkeyval* kv = headers.tqh_first;
    while(kv != nullptr){
      params.insert(std::make_pair(kv->key, kv->value));
      kv = kv->next.tqe_next;
    }

    return params;
  }

  /*
  // text/plain メッセージ送信
  static void sendRequest(struct evhttp_request *r, const std::string &message){
    struct evbuffer *evbuf;
    evbuf = evbuffer_new();
    if (evbuf == nullptr) {
      evhttp_send_error(r, HTTP_SERVUNAVAIL, "Failed to create buffer");
      return;
    }

    evhttp_add_header(r->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evbuffer_add(evbuf, message.c_str(), message.length());
    evhttp_send_reply(r, HTTP_OK, "", evbuf);
    evbuffer_free(evbuf);
  }
  */

  static void redirect(struct evhttp_request *r, const std::string &dest) {
    evhttp_add_header(r->output_headers, "Location", dest.c_str());
    evhttp_send_reply(r, HTTP_MOVETEMP, "Found", nullptr);
  }

  void server_error(const char* error_message){
    fprintf(stderr, "%s", error_message);
    exit(EXIT_FAILURE);
  }

private:
  struct event_base            *_base;
  struct evhttp                *_httpd;
  const char                   *_address;
  const int                     _port;
};

int main() {
  HttpServer server;
  server.start();
  return 0;
}

