/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Author : Maxim Mamontov <faust@stargazer.dp.ua>
 */

#include "parser_services.h"

#include "stg/services.h"

#include <strings.h> // strcasecmp

using STG::PARSER::GET_SERVICES;
using STG::PARSER::GET_SERVICE;
using STG::PARSER::ADD_SERVICE;
using STG::PARSER::DEL_SERVICE;
using STG::PARSER::CHG_SERVICE;

const char * GET_SERVICES::tag = "GetServices";
const char * GET_SERVICE::tag  = "AddService";
const char * ADD_SERVICE::tag  = "AddService";
const char * DEL_SERVICE::tag  = "DelService";
const char * CHG_SERVICE::tag  = "SetService";

void GET_SERVICES::CreateAnswer()
{
    // TODO: no priviledges implemented yet
    /*const PRIV * priv = m_currAdmin.GetPriv();
    if (!priv->serviceChg)
    {
        m_answer = "<Error Result=\"Error. Access denied.\"/>";
        return;
    }*/

    m_answer = "<Services>";
    SERVICE_CONF conf;
    int h = m_services.OpenSearch();
    while (m_services.SearchNext(h, &conf) == 0)
    {
        m_answer += "<Service name=\"" + conf.name +
                    "\" comment=\"" + Encode12str(conf.comment) +
                    "\" cost=\"" + x2str(conf.cost) +
                    "\" payDay=\"" + x2str(conf.payDay) + "\"/>";
    }
    m_services.CloseSearch(h);
    m_answer += "</Services>";
}

int GET_SERVICE::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        m_name = attr[1];
        return 0;
    }
    return -1;
}

void GET_SERVICE::CreateAnswer()
{
    // TODO: no priviledges implemented yet
    /*const PRIV * priv = m_currAdmin.GetPriv();
    if (!priv->serviceChg)
    {
        m_answer = "<Error Result=\"Error. Access denied.\"/>";
        return;
    }*/

    SERVICE_CONF conf;
    if (!m_services.Find(m_name, &conf))
        m_answer = "<Error result=\"Service '" + m_name + "' does not exist.\"/>";
    else
        m_answer += "<" + m_tag + " name=\"" + conf.name +
                    "\" comment=\"" + Encode12str(conf.comment) +
                    "\" cost=\"" + x2str(conf.cost) +
                    "\" payDay=\"" + x2str(conf.payDay) + "\"/>";
}

int ADD_SERVICE::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        m_name = attr[1];
        return 0;
    }
    return -1;
}

void ADD_SERVICE::CreateAnswer()
{
    SERVICE_CONF conf(m_name);
    if (m_services.Add(conf, &m_currAdmin) == 0)
        m_answer = "<" + m_tag + " result=\"Ok\"/>";
    else
        m_answer = "<" + m_tag + " result=\"" + m_services.GetStrError() + "\"/>";
}

int DEL_SERVICE::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        m_name = attr[1];
        return 0;
    }
    return -1;
}

void DEL_SERVICE::CreateAnswer()
{
    if (m_services.Del(m_name, &m_currAdmin) == 0)
        m_answer = "<" + m_tag + " result=\"Ok\"/>";
    else
        m_answer = "<" + m_tag + " result=\"" + m_services.GetStrError() + "\"/>";
}

int CHG_SERVICE::Start(void *, const char * el, const char ** attr)
{
    if (strcasecmp(el, m_tag.c_str()) == 0)
    {
        for (size_t i = 0; i < 8; i += 2)
        {
            if (attr[i] == NULL)
                break;

            if (strcasecmp(attr[i], "name") == 0)
            {
                m_service.name = attr[i + 1];
                continue;
            }

            if (strcasecmp(attr[i], "comment") == 0)
            {
                m_service.comment = Decode21str(attr[i + 1]);
                continue;
            }

            if (strcasecmp(attr[i], "cost") == 0)
            {
                double cost = 0;
                if (str2x(attr[i + 1], cost) == 0)
                    m_service.cost = cost;
                else
                    printfd(__FILE__, "Bad cast from '%s' to double\n", attr[i + 1]);
                // TODO: log it
                continue;
            }

            if (strcasecmp(attr[i], "payDay") == 0)
            {
                unsigned payDay;
                if (str2x(attr[i + 1], payDay) == 0)
                    m_service.payDay = payDay;
                else
                    printfd(__FILE__, "Bad cast from '%s' to unsigned\n", attr[i + 1]);
                // TODO: log it
                continue;
            }
        }

        return 0;
    }
    return -1;
}

void CHG_SERVICE::CreateAnswer()
{
    if (m_service.name.empty())
    {
        m_answer = "<" + m_tag + " result=\"Empty service name.\"/>";
        return;
    }

    if (!m_services.Exists(m_service.name.const_data()))
    {
        m_answer = "<" + m_tag + " result = \"Service '" + m_service.name.const_data() + "' does not exist.\"/>";
        return;
    }

    SERVICE_CONF orig;
    m_services.Find(m_service.name.const_data(), &orig);

    SERVICE_CONF_RES conf(orig);
    conf.Splice(m_service);

    if (m_services.Change(conf.GetData(), &m_currAdmin) != 0)
        m_answer = "<" + m_tag + " result = \"" + m_services.GetStrError() + "\"/>";
    else
        m_answer = "<" + m_tag + " result = \"Ok\"/>";
}
