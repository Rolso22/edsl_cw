//
// Created by ramagf on 15.02.23.
//

#ifndef EDSL_CW_ATTR_STACK_H
#define EDSL_CW_ATTR_STACK_H


struct AttrStackItemBase {
    AttrStackItemBase *next;

    AttrStackItemBase(AttrStackItemBase *next): next(next) {}

    virtual ~AttrStackItemBase() {}
};

template <typename T>
struct AttrStackItem : public AttrStackItemBase {
    T value;

    AttrStackItem(T value, AttrStackItemBase *next)
            : AttrStackItemBase(next)
            , value(value)
    {}
};

class AttrStack {
    AttrStackItemBase *stack;
public:
    AttrStack(): stack(nullptr) {}

    template <typename T>
    void push(T value) {
        stack = new AttrStackItem<T>(value, stack);
    }

    template <typename T>
    T pop() {
        auto top = dynamic_cast<AttrStackItem<T>*>(stack);
        stack = stack->next;
        T value = top->value;
        delete top;
        return value;
    }
};

struct ActionBase {
    virtual ~ActionBase() {}

    virtual void apply(AttrStack &stack) = 0;
};

#endif //EDSL_CW_ATTR_STACK_H
