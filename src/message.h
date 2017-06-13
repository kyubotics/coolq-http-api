#pragma once

#include "common.h"

#include "CQApp.h"

extern CQApp *CQ;

#define MSG_FMT_STRING "string"
#define MSG_FMT_ARRAY "array"

class Message {
public:
    Message(const str &msg_str) : msg_str_(msg_str), msg_json_(nullptr) {}

    Message(json_t *msg_json) : msg_str_(""), msg_json_(msg_json) {
        json_incref(msg_json);
    }

    Message(const Message &other) {
        this->msg_str_ = other.msg_str_;
        this->msg_json_ = other.msg_json_;
        json_incref(this->msg_json_);
    }

    /**
     * Convert message to a string, which can be sent directly (CQ codes will be enhanced as OUTCOMING).
     */
    str process_outcoming() const;

    /**
     * Convert received message to a json value in the specified format.
     * 
     * \param msg_fmt: the desired message format, if not passed in, use the one in config file
     */
    json_t *process_incoming(const str &msg_fmt = CQ->config.post_message_format) const;

private:
    str msg_str_;
    json_t *msg_json_;
};
