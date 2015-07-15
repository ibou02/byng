#include <iostream>
#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <pqxx/pqxx>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "mongoose.h"
#include "jsonxx.h"


using namespace AmqpClient;

const std::string EXCHANGE_NAME = "ByngCrawlerExchange";
const std::string QUEUE_NAME = "ByngCrawlerQueue";
const std::string ROUTING_KEY = "ByngCrawlerRouting";
const std::string CONSUMER_TAG = "ByngCrawlerConsumer";

Channel::ptr_t channel;
std::string queue;

pqxx::connection* c = 0;

static void handle_query(struct mg_connection *conn) {
    char query[500];
    char page[50];
    int pageInt;
    char limit[50];
    mg_send_header(conn, "Content-Type", "application/json");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    int result = mg_get_var(conn, "query", query, 500);
    if (result == -1 || result == -2) {
        mg_send_status(conn, 400);
        return;
    } else {
        int getPage = mg_get_var(conn, "page", page, 50);
        int getLimit = mg_get_var(conn, "limit", limit, 50);
        if (getPage == -1 || result == -2) {
            strcpy(page, "1");
        }
        if (getLimit == -1 || result == -2) {
            strcpy(limit, "15");
        }
        pageInt = atoi(page) * 15 - 15;
        pqxx::work search_query(*c);
        std::string queryParsed(query);
        boost::replace_all(queryParsed, " ", " & ");
        pqxx::result results = search_query.exec("SELECT url, title, favicon "
                                                 "FROM ("
                                                 "SELECT url as url, title as title, favicon as favicon, "
                                                 "setweight(to_tsvector(array_to_string(string_to_array(url, '-'), ',')), 'A') || ' ' ||"
                                                 "setweight(to_tsvector(title), 'B') || ' ' || "
                                                 "setweight(to_tsvector(array_to_string(h1, ',')), 'B') || ' ' || "
                                                 "setweight(to_tsvector(array_to_string(h2, ',')), 'C') || ' ' || "
                                                 "setweight(to_tsvector(array_to_string(h3, ',')), 'C') || ' ' || "
                                                 "setweight(to_tsvector(array_to_string(h4, ',')), 'D') || ' ' || "
                                                 "setweight(to_tsvector(array_to_string(h5, ',')), 'D') || ' ' || "
                                                 "setweight(to_tsvector(array_to_string(h6, ',')), 'D') "
                                                 "as document from crawled_sites) search "
                                                 "WHERE search.document @@ to_tsquery(" + search_query.quote(queryParsed) + ") ORDER BY ts_rank(search.document, to_tsquery(" + search_query.quote(queryParsed) + ")) DESC LIMIT " + limit + " OFFSET " + std::to_string(pageInt));
        jsonxx::Array json_items;
        for(pqxx::result::size_type i = 0; i != results.size(); ++i) {
            std::string title;
            std::string url;
            std::string favicon;
            results[i][0].to(url);
            results[i][1].to(title);
            results[i][2].to(favicon);
            jsonxx::Object element;
            element << "title" << title;
            element << "url" << url;
            element << "favicon" << favicon;
            json_items << element;
        }
        mg_printf_data(conn, json_items.json().c_str());
    }
    
}

static void handle_submit(struct mg_connection *conn) {
    char url[500];
    mg_send_header(conn, "Content-Type", "application/json");
    mg_send_header(conn, "Access-Control-Allow-Origin", "*");
    int result = mg_get_var(conn, "url", url, 500);
    if (result == -1 || result == -2) {
        mg_send_status(conn, 400);
        return;
    }
    channel->BasicPublish(EXCHANGE_NAME, ROUTING_KEY, BasicMessage::Create(url));
    mg_send_status(conn, 200);
    return;
}


static void send_404(struct mg_connection *conn) {
    mg_send_status(conn, 404);
    mg_printf_data(conn, "404. Not found.");
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
    if (ev == MG_REQUEST) {
        if (strcmp(conn->uri, "/query") == 0) {
            handle_query(conn);
        } else if (strcmp(conn->uri, "/submit") == 0) {
            handle_submit(conn);
        }
        else {
            send_404(conn);
        }

        return MG_TRUE;
    } else if (ev == MG_AUTH) {
        return MG_TRUE;
    } else {
        return MG_TRUE;
    }
}

std::string getEnv(const char* key) {
    char const* value = getenv(key);
    if (value == NULL) {
        return std::string();
    }
    else {
        return std::string(value);
    }
}

int main(int argc, char* argv[])
{
    using namespace std;
    string dbname = getEnv("DBNAME");
    string dbuser = getEnv("DBUSER");
    string dbpassword = getEnv("DBPASSWORD");
    string dbhost = getEnv("DBHOST");
    string dbport = getEnv("DBPORT");
    string listeningport = getEnv("LISTENING_PORT");

    string mqhost = getEnv("MQHOST");
    if (mqhost.empty())
    {
        mqhost.assign("172.17.8.254");
    }
    channel = Channel::Create(mqhost);
    channel->DeclareExchange(EXCHANGE_NAME, Channel::EXCHANGE_TYPE_FANOUT);
    queue = channel->DeclareQueue(QUEUE_NAME, false, true, false, false);
    channel->BindQueue(queue, EXCHANGE_NAME, ROUTING_KEY);
    
    if (dbname.empty())
    {
        cerr << "DBNAME env not set." << std::endl;
        return 1;
    }
    if (dbuser.empty())
    {
        cerr << "DBUSER env not set." << std::endl;
        return 1;
    }
    if (dbpassword.empty())
    {
        cerr << "DBPASSWORD env not set." << std::endl;
        return 1;
    }
    if (dbhost.empty())
    {
        cerr << "DBHOST env not set." << std::endl;
        return 1;
    }
    if (dbport.empty())
    {
        dbport = "5432";
    }
    if (listeningport.empty())
    {
        listeningport = "8080";
    }
    
    c = new pqxx::connection("dbname=" + dbname + " user=" + dbuser + " password=" + dbpassword + " host=" + dbhost + " port=" + dbport);
    struct mg_server *server = mg_create_server(NULL, ev_handler);
    mg_set_option(server, "document_root", ".");
    mg_set_option(server, "listening_port", listeningport.c_str());
    cout << "Server listening on " + listeningport << std::endl;
    for (;;) {
        mg_poll_server(server, 1000);
    }
    mg_destroy_server(&server);
    return 0;
}