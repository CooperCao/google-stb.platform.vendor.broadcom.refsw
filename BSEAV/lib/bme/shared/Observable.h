/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_COMMON_OBSERVABLE_H_
#define LIB_COMMON_OBSERVABLE_H_

#include <stdio.h>
#include <algorithm>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <utility>  // for std::forward

using namespace std::placeholders;  // for _1, _2 etc.

#define OBSERVER_TYPE std::function<void(Args...)>

template <typename Event = int>
class Observable
{
    public:
        Observable() {}
        virtual ~Observable() {}

        template<typename F>
            std::shared_ptr<void> addListener(const Event& event, F& observer)
            {
                Wrap w(observer);
                _observers[event].push_back(w);
                return w.getId();
            }

            void removeListener(const Event& event, std::shared_ptr<void>& reg)
            {
                auto v = _observers.at(event);
                v.erase(std::remove_if(v.begin(), v.end(), [&](Wrap wrap) {
                            return (wrap.getId() == reg);}),
                        v.end());
                _observers[event] = v;
            }

        template <typename... Args>
            void notify(const Event& event, Args... args) const
            {
                if (_observers.count(event) > 0 )
                {
                    auto v = _observers.at(event);

                    for ( auto i = v.begin(); i != v.end(); i++ ) {
                        auto observer = *i;
                        OBSERVER_TYPE fn = observer.template get<OBSERVER_TYPE>();
                        fn(args...);
                    };
                };
            }

        // Do not allow copying or assigning
        Observable(const Observable&)=delete;
        Observable& operator=(const Observable&)=delete;

    private:
        //////////////////////////////////////////////////////////////////////////////////////////
        // Wrap: Wraps an observer object into a common type class. Implements type erasure such
        //       that the wrapper can be used to construct a heterogenous vector of observers
        class Wrap {
            public:
                template< typename T >
                    explicit Wrap(const T& obj) : _concept(std::make_shared<Model<T>>(obj)) {}
                virtual ~Wrap() {}

                template<typename T>
                    T const& get() const {
                        return std::static_pointer_cast<Model<T>>(_concept)->data;
                    }

                std::shared_ptr<void> getId() {
                    return _concept;
                }

            private:
                struct Concept {
                    virtual ~Concept() {}
                };

                template< typename T >
                    struct Model : Concept {
                        explicit Model(const T& t) : data(t) {}
                        virtual ~Model() {}
                        T data;
                    };

                std::shared_ptr<Concept> _concept;
        };
        //End Wrap ///////////////////////////////////////////////////////////////////////////////

        std::map<Event, std::vector<Wrap>> _observers;
};

#endif  // LIB_COMMON_OBSERVABLE_H_
