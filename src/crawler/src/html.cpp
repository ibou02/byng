#include <iostream>
#include <gumbo.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <Poco/URI.h>
#include "easylogging++.h"

void search_for_links(GumboNode* node, std::vector<std::string>* results, Poco::URI originUrl) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        Poco::URI hrefUrl(href->value);
        if (hrefUrl.isRelative()) {
            Poco::URI newUrl(originUrl, href->value);
            results->push_back(newUrl.toString().c_str());
        } else {
            results->push_back(href->value);
        }
    }
    
    GumboVector* children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i) {
        search_for_links(static_cast<GumboNode*>(children->data[i]), results, originUrl);
    }
}

std::string get_title(GumboNode* root) {
    assert(root->type == GUMBO_NODE_ELEMENT);
    assert(root->v.element.children.length >= 2);
    
    // Search for HEAD element
    GumboVector* children = &root->v.element.children;
    GumboNode* head = NULL;
    for (int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*>(children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        };
    }
    
    assert(head != NULL);
    
    GumboVector* head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*>(head_children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_TITLE) {
            if (child->v.element.children.length != 1) {
                return "<empty title>";
            }
            GumboNode* title_text = static_cast<GumboNode*>(child->v.element.children.data[0]);
            assert(title_text->type == GUMBO_NODE_TEXT);
            return title_text->v.text.text;
        }
    }
    return "<no title found>";
}

std::string get_favicon(GumboNode* root) {
    assert(root->type == GUMBO_NODE_ELEMENT);
    assert(root->v.element.children.length >= 2);
    
    // Search for HEAD element
    GumboVector* children = &root->v.element.children;
    GumboNode* head = NULL;
    for (int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*>(children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        };
    }
    
    assert(head != NULL);
    
    GumboVector* head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i)
    {
        GumboNode* child = static_cast<GumboNode*>(head_children->data[i]);
        GumboAttribute* rel_attribute;
        if (child->type == GUMBO_NODE_ELEMENT
            && child->v.element.tag == GUMBO_TAG_LINK
            && (rel_attribute = gumbo_get_attribute(&child->v.element.attributes, "rel"))
            && strstr(rel_attribute->value, "icon"))
        {
            GumboAttribute* href_attribute = gumbo_get_attribute(&child->v.element.attributes, "href");
            return href_attribute->value;
        }
    }
    return "";
}

void search_tag(GumboNode* node, GumboTag tag, std::vector<std::string>* elements) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    
    if (node->v.element.tag == tag) {
        if (node->v.element.children.length == 1) {
            GumboNode* header_text = static_cast<GumboNode*>(node->v.element.children.data[0]);
            if (header_text->type == GUMBO_NODE_TEXT) {
                elements->push_back(std::string(header_text->v.text.text));
            }
        }
    } else {
        GumboVector* children = &node->v.element.children;
        for (int i = 0; i < children->length; ++i)
        {
            search_tag(static_cast<GumboNode*>(children->data[i]), tag, elements);
        }
    }
}