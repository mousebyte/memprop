# memprop
Memprop is a member property implementation suitable for use in UI libraries. Features change notifications, access control, and bindings. Requires a compiler with C++20 Concepts support.

## Installation
Memprop is a header only library. It depends on [palacaze/sigslot](https://github.com/palacaze/sigslot) for property change notifications. As long as you have `sigslot/signal.hpp` available in your project, all you need is the Memprop include directory.
### Install with CMake
You can install Memprop either as a subdirectory/submodule of your project, or to the system using the install target.
First, clone the Memprop repo using `git clone` or `git submodule add`. Then update memprop's submodules. If you're not interested in running the test cases, the only one you'll need is sigslot.
```
cd memprop
git submodule update --init ./lib/sigslot
```
If you're using memprop as a subdirectory of your project, just add it to your `CMakeLists.txt`:
```cmake
option(MEMPROP_COMPILE_TESTS "" OFF)
add_subdirectory(memprop)
```
Otherwise, you can install it wherever you like.
```
mkdir build
cd build
cmake .. -B. -DMEMPROP_COMPILE_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build . --target install
```

## Usage
### Public properties
There are a number of different property types to choose from based on use case. The simplest property type is `public_property`, which any caller can get or set. The first template argument to any property is always the type of the owning class. Getters and setters are provided as member function pointers.
```c++
class foo {
    bool set_StringProp(std::string& out, std::string const& in)
        {
        out = in.substr(0, 32); //perform some custom processing or validation
        return true;            //return false to suppress Changed signal
        }
        
public:
    //Public property with default setter and getter
    memprop::public_property<foo, int> IntProp {this};
    //Public property with custom setter
    memprop::public_property<foo, int, &foo::set_StringProp> StringProp;
    
    foo() : StringProp(this, "bar") //Non-backed property constructors can accept an initial value
        {
        }
};
```
### Readonly properties
Readonly properties are identical to public properties, but can only be set from within the owner type.
```c++
class foo {
public:
    memprop::readonly_property<foo, int> RoProp {this};
    
    void bar()
        {
        RoProp = 17; //Can be set from inside foo
        }
};

void baz(foo& f)
    {
    int i = f.RoProp; //Can read the value from outside
    //f.RoProp = 22;    Error: produces an access violation
    }
```
### Backed properties
Both public and readonly properties have variants which use a backing field. These properties require a custom getter and setter.
```c++
class foo {
    int _anInt;
    
    int const& get_int() const  //Getter returns a const reference
        {
        return _anInt;
        }
        
    bool set_int(int const& v)  //Setter for backed property takes a const reference
        {
        if(v <= 100) {
            _anInt = v;
            return true;
            }
        return false;
        }
        
public:
    memprop::backed_public_property<foo, int, &foo::get_int, &foo::set_int> BackedProp {this};
};
```
### Computed properties
The last type of property is the `computed_property`, which has a custom getter and no setter whatsoever.
```c++
class foo {
    float magic_number() const
        {
        //do some magic
        }
        
public:
    memprop::computed_property<foo, float, &foo::magic_number> ComputedProp {this};
};
```
### Change notifications
All property types except `computed_property` have a member signal, `Changed`, which is invoked each time the property's value is set from the property object. A const reference to the property's new value is passed to each slot. For more information on the signals used in this library, check out the [sigslot](https://github.com/palacaze/sigslot) repo.
### Property binding
A property can be bound to the value of another property with the `bind()` member function. Readonly properties can only be bound to the value of another property from within their owner class. The only property type which does not support binding is `computed_property`.

A custom converter object can be passed to `bind()`. Converters must have an `operator()` which expects a const reference to the source value type, and returns the target value type.
```c++
#include <string>

class foo {
public:
    memprop::public_property<foo, int> FooIntProp {this};
    memprop::public_property<foo, std::string> FooStringProp {this};
};

class bar {
public:
    memprop::public_property<bar, int> BarIntProp {this};
    memprop::readonly_property<bar, int> BarReadonlyIntProp {this};
};

struct custom_converter {
    std::string operator()(int const& v)
        {
        return std::to_string(v);
        }
};

void do_some_binding()
    {
    foo f;
    bar b;
    
    //FooIntProp will now be set whenever BarReadonlyIntProp is set
    auto binding = f.FooIntProp.bind(b.BarReadonlyIntProp);
    
    //b.BarReadonlyIntProp.bind(f.FooIntProp);  Error: can't bind to readonly property outside of bar
    
    //Bindings can be managed through the handle returned by bind()
    binding->disconnect();
    
    //Bindings can be bi-directional, setting either property will update the other
    f.FooIntProp.bind(b.BarIntProp);
    b.BarIntProp.bind(f.FooIntProp);
    
    //Bindings can be unset through the property
    f.FooIntProp.unbind();
    
    //a custom converter can be passed to modify the incoming value
    f.FooStringProp.bind(b.BarIntProp, custom_converter{});
    }
```
