/*
 *    Author : Boris Mikhailenko <stg34@stargazer.dp.ua>
 */

#pragma once

#include <string>
#include <ctime>
#include <cstdint>

namespace STG
{

struct Message
{
    struct Header
    {
        Header() noexcept
            : id(0),
              ver(0),
              type(0),
              lastSendTime(0),
              creationTime(0),
              showTime(0),
              repeat(0),
              repeatPeriod(0)
        {}

        Header(const Header&) = default;
        Header& operator=(const Header&) = default;
        Header(Header&&) = default;
        Header& operator=(Header&&) = default;

        uint64_t    id;
        unsigned    ver;
        unsigned    type;
        unsigned    lastSendTime;
        unsigned    creationTime;
        unsigned    showTime;
        int         repeat;
        unsigned    repeatPeriod;
    };

    Message() = default;

    Message(const Message&) = default;
    Message& operator=(const Message&) = default;
    Message(Message&&) = default;
    Message& operator=(Message&&) = default;

    time_t GetNextSendTime() const
    {
        return header.lastSendTime + header.repeat * 60;
    }

    Header header;
    std::string text;
};

}
