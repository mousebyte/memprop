# memprop
Memprop is a member property implementation suitable for use in UI libraries. Features change notifications, access control, and bindings. Requires a compiler with C++20 Concepts support.

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
### Property binding
A property can be bound to the value of another property. To-do: finish binding section
