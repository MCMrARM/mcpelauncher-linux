#pragma once

#include <functional>

namespace mcpe {

template<typename T>
class function;

template<typename ReturnType, typename... ArgTypes>
class function<ReturnType(ArgTypes...)> {

    class AnyClass;

    union AnyValue
    {
        void* object;
        const void* objectConst;
        void (*function)();
        void (AnyClass::*memberFunction)();
    };

    AnyValue functor;


    enum class ManagerOperation
    {
        GetTypeInfo,
        GetFunctor,
        CloneFunctor,
        DestroyFunctor
    };

    typedef bool (*ManagerFunc)(AnyValue&, const AnyValue&, ManagerOperation);

    ManagerFunc manager;


    using Invoker = ReturnType (*)(const AnyValue&, ArgTypes&&...);

    Invoker invoker;


    void destroy() {
        if (manager != nullptr)
            manager(functor, functor, ManagerOperation::DestroyFunctor);
    }


public:

    function() : manager(nullptr) {}

    function(function const& func) {
        manager = nullptr;
        *this = func;
    }

    function(function&& func) {
        manager = nullptr;
        *this = std::move(func);
    }

    ~function() { destroy(); }

    function& operator=(function const& func) {
        destroy();
        manager = func.manager;
        if (func) {
            manager(functor, func.functor, ManagerOperation::CloneFunctor);
            invoker = func.invoker;
        }
        return *this;
    }

    function& operator=(function&& func) {
        destroy();
        functor = func.functor;
        manager = func.manager;
        invoker = func.invoker;
        func.functor.object = nullptr;
        func.manager = nullptr;
        func.invoker = nullptr;
        return *this;
    }

    explicit operator bool() const { return manager != nullptr; }

    ReturnType operator()(ArgTypes... args) const {
        if (manager == nullptr)
            throw std::bad_function_call();
        return invoker(functor, std::forward(args)...);
    }

};

}
