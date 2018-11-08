#ifndef LMR_REGISTER_H
#define LMR_REGISTER_H

#include <unordered_map>
#include <string>


namespace lmr
{
    using namespace std;

#define BASE_CLASS_REGISTER(base_class)                                 \
    class BaseClassRegister_##base_class {                              \
    public:                                                             \
        typedef base_class* (*Creator)();                               \
                                                                        \
        void add_creator(string class_name, Creator creator) {          \
            um_creator[class_name] = creator;                           \
        }                                                               \
                                                                        \
        base_class* create_object(const string& class_name){            \
            if (um_creator.count(class_name))                           \
                return um_creator[class_name]();                        \
            else                                                        \
                return nullptr;                                         \
        }                                                               \
        static BaseClassRegister_##base_class* get_instance() {         \
            if (instance)                                               \
                return instance;                                        \
            else                                                        \
                return instance = new BaseClassRegister_##base_class;   \
        }                                                               \
    private:                                                            \
        static BaseClassRegister_##base_class* instance;                \
        BaseClassRegister_##base_class() {}                             \
        ~BaseClassRegister_##base_class() {                             \
            if (instance){                                              \
                delete instance;                                        \
                instance = nullptr;                                     \
            }                                                           \
        }                                                               \
        unordered_map<string, Creator> um_creator;                      \
    };                                                                  \
    BaseClassRegister_##base_class*                                     \
        BaseClassRegister_##base_class::instance = nullptr;             \
                                                                        \
    class BaseClassRegisterAdder_##base_class {                         \
    public:                                                             \
        BaseClassRegisterAdder_##base_class(                            \
            const string& class_name,                                   \
            BaseClassRegister_##base_class::Creator creator) {          \
            BaseClassRegister_##base_class::get_instance()->add_creator(\
                class_name, creator);                                   \
        }                                                               \
    };


#define CHILD_CLASS_REGISTER(base_class, child_class)                   \
    base_class* ObjectCreator_##base_class##child_class() {             \
        return new child_class;                                         \
    }                                                                   \
    BaseClassRegisterAdder_##base_class                                 \
    g_child_class_adder_##base_class##child_class(                      \
        #child_class, ObjectCreator_##base_class##child_class);


#define CHILD_CLASS_CREATOR(base_class, child_class)                    \
    BaseClassRegister_##base_class::                                    \
        get_instance()->create_object(child_class)

}

#endif //LMR_REGISTER_H
