#include <ctime>
#include <iostream>
#include <boost/atomic.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/thread.hpp>
#include <curl/curl.h>
#include <gumbo.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <pqxx/pqxx>
#include "html.h"
#include "easylogging++.h"

using namespace AmqpClient;
using namespace pqxx;

_INITIALIZE_EASYLOGGINGPP

boost::lockfree::spsc_queue<std::string, boost::lockfree::capacity<1024> > spsc_queue;


std::string getEnv(const char* key) {
    char const* value = getenv(key);
    if (value == NULL) {
        return std::string();
    }
    else {
        return std::string(value);
    }
}

std::string data;

const std::string EXCHANGE_NAME = "ByngCrawlerExchange";
const std::string QUEUE_NAME = "ByngCrawlerQueue";
const std::string ROUTING_KEY = "ByngCrawlerRouting";
const std::string CONSUMER_TAG = "ByngCrawlerConsumer";

std::string dbname = getEnv("DBNAME");
std::string dbuser = getEnv("DBUSER");
std::string dbpassword = getEnv("DBPASSWORD");
std::string dbhost = getEnv("DBHOST");
std::string dbport = getEnv("DBPORT");
std::string threads = getEnv("THREADS");
std::string mqhost = getEnv("MQHOST");

Channel::ptr_t channel;
std::string queue;


size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) {
    //buf is a pointer to the data that curl has for us
    //size*nmemb is the size of the buffer
    
    for (int c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size*nmemb;
}

std::string toPGArray(std::vector<std::string> elements, connection* c) {
    std::string pgArray = "{";
    for (int i = 0; i < elements.size(); i++) {
        pgArray += "\"" + c->esc(elements[i]) + "\",";
    }
    if (elements.size() > 0) {
        pgArray.pop_back();
    }
    pgArray += "}";
    return pgArray;
}

void scheduler(void)
{
    LOG(INFO) << "Scheduling links";
    Channel::ptr_t channelfoo = Channel::Create(mqhost);
    channelfoo->DeclareExchange(EXCHANGE_NAME, Channel::EXCHANGE_TYPE_FANOUT);
    std::string queuefoo = channel->DeclareQueue(QUEUE_NAME, false, true, false, false);
    channelfoo->BindQueue(queuefoo, EXCHANGE_NAME, ROUTING_KEY);
    connection c("dbname=" + dbname + " user=" + dbuser + " password=" + dbpassword + " host=" + dbhost + " port=" + dbport);
    work ScheduleAction(c, "ScheduleAction");
    result results = ScheduleAction.exec("SELECT url FROM crawled_sites WHERE crawled_at <= NOW() - '1 day'::INTERVAL LIMIT 15");
    for(pqxx::result::size_type i = 0; i != results.size(); ++i) {
        std::string url;
        results[i][0].to(url);
        channel->BasicPublish(EXCHANGE_NAME, ROUTING_KEY, BasicMessage::Create(url));
    }
    LOG(INFO) << "Scheduled " + std::to_string(results.size()) + " links";
    boost::this_thread::sleep(boost::posix_time::minutes(5));
    
}

void crawler(void)
{
    LOG(INFO) << "HI";
    channel->BasicConsume(queue, CONSUMER_TAG);
    Envelope::ptr_t env;
    connection c("dbname=" + dbname + " user=" + dbuser + " password=" + dbpassword + " host=" + dbhost + " port=" + dbport);
    
    while (true) {
        if (channel->BasicConsumeMessage(CONSUMER_TAG, env, 0)) {
            std::string url = env->Message()->Body();
            Poco::URI originUrlParsed;
            try {
                originUrlParsed = Poco::URI(url);
            }
            catch (Poco::SyntaxException e) {
                continue;
            }
            work ExistsAction(c, "ExistsAction");
            bool toCrawl = true;
            result results = ExistsAction.exec("SELECT extract(epoch from crawled_at) FROM crawled_sites WHERE url = " + ExistsAction.quote(url));
            if (results.size() != 0) {
                try {
                    if (static_cast<int>(std::time(NULL)) - results[0][0].as<int>() < 86400) {
                        toCrawl = false;
                    }
                } catch (conversion_error e) {
                    toCrawl = true;
                }
            }
            ExistsAction.abort();
            if (toCrawl) {
                LOG(INFO) << "Crawling " << url;
                CURL *curl;
                curl_global_init(CURL_GLOBAL_ALL);
                CURLcode res;
                curl = curl_easy_init();
                data = "";
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "byng-crawler 162424@supinfo.com");
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
                //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                GumboOutput* output = gumbo_parse(data.c_str());
                std::string title = get_title(output->root);
                std::string favicon = get_favicon(output->root);
                std::vector<std::string> h1;
                std::vector<std::string> h2;
                std::vector<std::string> h3;
                std::vector<std::string> h4;
                std::vector<std::string> h5;
                std::vector<std::string> h6;
                std::vector<std::string> urls;
                search_tag(output->root, GUMBO_TAG_H1, &h1);
                search_tag(output->root, GUMBO_TAG_H1, &h1);
                search_tag(output->root, GUMBO_TAG_H2, &h2);
                search_tag(output->root, GUMBO_TAG_H3, &h3);
                search_tag(output->root, GUMBO_TAG_H4, &h4);
                search_tag(output->root, GUMBO_TAG_H5, &h5);
                search_tag(output->root, GUMBO_TAG_H6, &h6);
                search_for_links(output->root, &urls, originUrlParsed);
                for (int i = 0; i < urls.size(); i++) {
                    channel->BasicPublish(EXCHANGE_NAME, ROUTING_KEY, BasicMessage::Create(urls[i]));
                }
                gumbo_destroy_output(&kGumboDefaultOptions, output);
                try
                {
                    int currentTime = std::time(NULL);
                    std::stringstream timeStream;
                    timeStream << currentTime;
                    work SaveAction(c, "SaveTransaction");
                    std::string query = "WITH upsert AS (UPDATE crawled_sites SET "
                    "title=" + SaveAction.quote(title) + ", " +
                    "favicon=" + SaveAction.quote(favicon) + ", " +
                    "h1=" + SaveAction.quote(toPGArray(h1, &c)) + ", " +
                    "h2=" + SaveAction.quote(toPGArray(h2, &c)) + ", " +
                    "h3=" + SaveAction.quote(toPGArray(h3, &c)) + ", " +
                    "h4=" + SaveAction.quote(toPGArray(h4, &c)) + ", " +
                    "h5=" + SaveAction.quote(toPGArray(h5, &c)) + ", " +
                    "h6=" + SaveAction.quote(toPGArray(h6, &c)) + " WHERE url=" + SaveAction.quote(url) + " RETURNING *) " +
                    "INSERT INTO crawled_sites(url, title, favicon, crawled_at, h1, h2, h3, h4, h5, h6) "
                    "SELECT " +
                    SaveAction.quote(url) + ", " +
                    SaveAction.quote(title) + ", " +
                    SaveAction.quote(favicon) + ", " +
                    "to_timestamp(" + timeStream.str() + ")::timestamp, " +
                    SaveAction.quote(toPGArray(h1, &c)) + ", " +
                    SaveAction.quote(toPGArray(h2, &c)) + ", " +
                    SaveAction.quote(toPGArray(h3, &c)) + ", " +
                    SaveAction.quote(toPGArray(h4, &c)) + ", " +
                    SaveAction.quote(toPGArray(h5, &c)) + ", " +
                    SaveAction.quote(toPGArray(h6, &c)) + " " +
                    "WHERE NOT EXISTS (SELECT * FROM upsert);";
                    SaveAction.exec(query);
                    SaveAction.commit();
                }
                catch (failure e)
                {
                    throw e;
                }

            }
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    }
}

int main(int argc, char* argv[])
{
    using namespace std;
    
    LOG(INFO) << "Starting up byng-crawler";
    
    if (dbport.empty()) {
        dbport.assign("5432");
    }

    if (mqhost.empty())
    {
        mqhost.assign("localhost");
    }
    
    channel = Channel::Create(mqhost);
    channel->DeclareExchange(EXCHANGE_NAME, Channel::EXCHANGE_TYPE_FANOUT);
    queue = channel->DeclareQueue(QUEUE_NAME, false, true, false, false);
    channel->BindQueue(queue, EXCHANGE_NAME, ROUTING_KEY);
    
    if (threads.empty())
    {
        threads.assign("1");
    }
    
    boost::thread scheduler_thread(scheduler);
    boost::thread_group group;
    for (int i = 0; i < atoi(threads.c_str()); ++i)
        group.create_thread(crawler);
    group.join_all();
    scheduler_thread.join();
}