// Copyright (c) 2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "Url.h"

#include <iostream>
#include <sstream>

#include <glib.h>

struct Url::Impl {
	Impl(const std::string &uri);
    Impl(const Impl &o) = default;
    bool empty() const {
        return original.empty()
            || (scheme.empty()
                && host.empty()
                && path.empty());
    }
    std::string query() const;

    std::string original, scheme, host, path;
    std::map<std::string, std::string> queryMap;
};

Url::Url(const Url &u)
    : impl_(new Impl(*u.impl_))
{
}

Url &Url::operator=(const Url &otherUrl)
{
    *impl_ = *otherUrl.impl_;
    return *this;
}

// Move ctor and assignment op
// As we use unique_ptr, we can used default
// compiler generated for both here.
Url::Url(Url &&u) noexcept = default;

Url &Url::operator=(Url &&otherUrl) noexcept = default;

Url::~Url() = default;

Url::Url(const std::string &uri) : impl_(new Url::Impl(uri))
{
}

Url Url::fromLocalFile(const std::string &path)
{
    GError *err = NULL;
    char *uri = g_filename_to_uri(path.c_str(), NULL, &err);
    if (err != NULL || uri == NULL) {
        g_error_free(err);
        return Url();
    }
    Url ret(uri);
    free(uri);
    return ret;
}

std::string Url::scheme() const
{
    return impl_ ? impl_->scheme : std::string();
}

std::string Url::host() const
{
    return impl_ ? impl_->host : std::string();
}

std::string Url::path() const
{
    return impl_ ? impl_->path : std::string();
}

bool Url::empty() const
{
    return impl_ ? impl_->empty() : true;
}

bool Url::isLocalFile() const
{
    return scheme() == "file";
}

std::string Url::toString() const
{
    if (!impl_)
        return {};

    std::stringstream s;
    s << impl_->original << impl_->query();
    return s.str();
}

std::string Url::toLocalFile() const
{
    if (!impl_)
        return {};

    GError *err = NULL;
    char *cpath = g_filename_from_uri(impl_->original.c_str(), NULL, &err);
    if (err || !cpath) {
        g_error_free(err);
        return {};
    }
    std::string path(cpath);
    free(cpath);
    return path;
}

// TODO: Current impl supports setting query params only through
// this method. Url(std::string) constructor does not parse url
// params correctly yet.
bool Url::setQuery(const std::map<std::string, std::string> &query)
{
    std::map<std::string, std::string> escMap;
    char *escapedKey = NULL;
    char *escapedVal = NULL;
    for (const auto &q : query) {
        escapedKey = g_uri_escape_string(q.first.c_str(), NULL, TRUE);
        escapedVal = g_uri_escape_string(q.second.c_str(), NULL, TRUE);
        if (!escapedKey || !escapedVal)
            goto err;
        escMap.emplace(std::string(escapedKey), std::string(escapedVal));
    }
    if (impl_) {
        impl_->queryMap = escMap;
        return true;
    }
err:
   if (escapedKey) free(escapedKey);
   if (escapedVal) free(escapedVal);
   return false;
}

std::string Url::Impl::query() const
{
    if (queryMap.empty())
        return {};

    std::stringstream qs;
    for (const auto &q : queryMap)
        qs << '&' << q.first << '=' << q.second;
    auto result = qs.str();
    result[0] = '?';
    return result;
}

// Parses the URI paramater using GLib URI parsing functions.
//
// TODO: GLib URI parsing functions are limited to "local" URIs
// port, user credentials and even query parameters are not handled.
// Consider in the future move away from GLib and implement our own
// parsing code or use a third party impl, when available (boost not
// available yet, CURL has an experimental API as of version 7.62 [1])
//
// [1] https://daniel.haxx.se/blog/2018/09/09/libcurl-gets-a-url-api
Url::Impl::Impl(const std::string &uri)
{
    const char *uristr = uri.c_str();
    char *s, *h, *p;
    s = h = p = NULL;
    GError *err = NULL;

    original.assign(uri);
    s = g_uri_parse_scheme(uristr);
    if (!s)
        goto done;
    scheme.assign(std::string(s));

    p = g_filename_from_uri(uristr, &h, &err);
    if (err != NULL)
        goto done;

    if (p) path.assign(std::string(p));
    if (h) host.assign(std::string(h));

done:
    g_clear_error(&err);
    if (s) free(s);
    if (h) free(h);
    if (p) free(p);
}

