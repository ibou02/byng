#include <iostream>
#include <stdlib.h>
#include <gumbo.h>
#include <vector>
#include <Poco/URI.h>

void search_for_links(GumboNode* node, std::vector<std::string>* results, Poco::URI originUrl);

std::string get_title(GumboNode* node);

std::string get_favicon(GumboNode* node);

void search_tag(GumboNode* node, GumboTag tag, std::vector<std::string>* elements);