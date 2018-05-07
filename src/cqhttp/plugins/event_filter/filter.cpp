#include "./filter.h"

#include <regex>

using namespace std;

namespace cqhttp::plugins {
    static shared_ptr<IFilter> construct_op(const string &op_name, const json &op_argument);

    class NotOperator : public IFilter {
    public:
        static shared_ptr<NotOperator> construct(const json &argument) {
            if (!argument.is_object()) {
                throw FilterSyntexError("the argument of 'not' operator must be an object");
            }
            auto op = make_shared<NotOperator>();
            op->operand_ = construct_op("and", argument);
            return op;
        }

        bool eval(const json &payload) override { return !operand_->eval(payload); }

    private:
        shared_ptr<IFilter> operand_;
    };

    class AndOperator : public IFilter {
    public:
        static shared_ptr<AndOperator> construct(const json &argument) {
            if (!argument.is_object()) {
                throw FilterSyntexError("the argument of 'and' operator must be an object");
            }

            auto op = make_shared<AndOperator>();

            for (auto it = argument.begin(); it != argument.end(); ++it) {
                const auto &key = it.key();
                const auto &value = it.value();

                if (key.empty()) {
                    continue;
                }

                if (key.front() == '.') {
                    // is an operator
                    //   ".foo": {
                    //       "bar": "baz"
                    //   }
                    op->operands_.push_back({"", construct_op(key.substr(1), value)});
                } else if (value.is_object()) {
                    // is an normal key with an object as the value
                    //   "foo": {
                    //       ".bar": "baz"
                    //   }
                    op->operands_.push_back({key, construct_op("and", value)});
                } else {
                    // is an normal key with a non-object as the value
                    //   "foo": "bar"
                    op->operands_.push_back({key, construct_op("eq", value)});
                }
            }

            return op;
        }

        bool eval(const json &payload) override {
            auto res = true;

            for (const auto &operand : operands_) {
                if (operand.first.empty()) {
                    // is an operator
                    res = res && operand.second->eval(payload);
                } else {
                    // is an normal key
                    try {
                        auto &sub_payload = payload.at(operand.first);
                        res = res && operand.second->eval(sub_payload);
                    } catch (exception &) {
                        res = false;
                    }
                }

                if (res == false) {
                    break;
                }
            }

            return res;
        }

    private:
        vector<pair<string, shared_ptr<IFilter>>> operands_;
    };

    class OrOperator : public IFilter {
    public:
        static shared_ptr<OrOperator> construct(const json &argument) {
            if (!argument.is_array()) {
                throw FilterSyntexError("the argument of 'or' operator must be an array");
            }

            auto op = make_shared<OrOperator>();

            for (auto &elem : argument) {
                op->operands_.push_back(construct_op("and", elem));
            }

            return op;
        }

        bool eval(const json &payload) override {
            auto res = false;

            for (const auto &operand : operands_) {
                res = res || operand->eval(payload);

                if (res == true) {
                    break;
                }
            }

            return res;
        }

    private:
        vector<shared_ptr<IFilter>> operands_;
    };

    class EqualOperator : public IFilter {
    public:
        static shared_ptr<EqualOperator> construct(const json &argument) {
            auto op = make_shared<EqualOperator>();
            op->value_ = argument;
            return op;
        }

        bool eval(const json &payload) override { return payload == value_; }

    private:
        json value_;
    };

    class NotEqualOperator : public IFilter {
    public:
        static shared_ptr<NotEqualOperator> construct(const json &argument) {
            auto op = make_shared<NotEqualOperator>();
            op->value_ = argument;
            return op;
        }

        bool eval(const json &payload) override { return payload != value_; }

    private:
        json value_;
    };

    class InOperator : public IFilter {
    public:
        static shared_ptr<InOperator> construct(const json &argument) {
            if (!(argument.is_string() || argument.is_array())) {
                throw FilterSyntexError("the argument of 'in' operator must be a string or an array");
            }

            auto op = make_shared<InOperator>();
            op->range_ = argument;
            return op;
        }

        bool eval(const json &payload) override {
            if (range_.is_string()) {
                return payload.is_string() && boost::algorithm::contains(range_.get<string>(), payload.get<string>());
            }

            return find(range_.begin(), range_.end(), payload) != range_.end();
        }

    private:
        json range_;
    };

    class ContainsOperator : public IFilter {
    public:
        static shared_ptr<ContainsOperator> construct(const json &argument) {
            if (!argument.is_string()) {
                throw FilterSyntexError("the argument of 'contains' operator must be a string");
            }

            auto op = make_shared<ContainsOperator>();
            op->test_ = argument;
            return op;
        }

        bool eval(const json &payload) override {
            if (!payload.is_string()) {
                return false;
            }
            return boost::algorithm::contains(payload.get<string>(), test_.get<string>());
        }

    private:
        json test_;
    };

    class RegexOperator : public IFilter {
    public:
        static shared_ptr<RegexOperator> construct(const json &argument) {
            if (!argument.is_string()) {
                throw FilterSyntexError("the argument of 'regex' operator must be a string");
            }

            auto op = make_shared<RegexOperator>();
            op->regex_ = regex(argument.get<string>());
            return op;
        }

        bool eval(const json &payload) override {
            if (!payload.is_string()) {
                return false;
            }

            smatch m;
            const auto input = payload.get<string>();
            return regex_search(input.cbegin(), input.cend(), m, regex_);
        }

    private:
        regex regex_;
    };

    static shared_ptr<IFilter> construct_op(const string &op_name, const json &op_argument) {
        static const map<string, function<shared_ptr<IFilter>(const json &)>> op_constructor_map = {
            {"not", NotOperator::construct},
            {"and", AndOperator::construct},
            {"or", OrOperator::construct},
            {"eq", EqualOperator::construct},
            {"neq", NotEqualOperator::construct},
            {"in", InOperator::construct},
            {"contains", ContainsOperator::construct},
            {"regex", RegexOperator::construct},
        };

        if (op_constructor_map.find(op_name) == op_constructor_map.end()) {
            throw FilterSyntexError("the operator '" + op_name + "'" + "is not supported");
        }

        return op_constructor_map.at(op_name)(op_argument);
    }

    shared_ptr<IFilter> construct_filter(const json &root_filter) { return construct_op("and", root_filter); }
} // namespace cqhttp::plugins
