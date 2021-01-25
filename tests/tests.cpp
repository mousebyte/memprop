#include "memprop/memprop.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
using namespace mousebyte::memprop;

struct test_prop_type {
    int i;
};


class test_class {
    bool set_intprop2(
        int&       o,
        int const& i
        )
        {
        auto success = i < 10;

        if (success) o = i;
        return success;
        }

    char const& get_char() const
        {
        return _backingChar;
        }

    bool set_char(
        char const& c
        )
        {
        _backingChar = c;
        return true;
        }

    std::string const& get_str() const
        {
        return _backingString;
        }

    bool set_str(
        std::string const& str
        )
        {
        _backingString = str;
        return true;
        }

    int compute_prop() const
        {
        return _computeAdd + 3;
        }

public:
    char _backingChar;
    std::string _backingString;
    int _computeAdd;
    public_property<test_class, int> IntProp1;
    public_property<test_class, int, &test_class::set_intprop2> IntProp2;
    backed_public_property<test_class, char, &test_class::get_char,
                           &test_class::set_char> CharProp1;
    readonly_property<test_class, std::string> StringProp1;
    backed_readonly_property<test_class, std::string, &test_class::get_str,
                             &test_class::set_str> StringProp2;
    computed_property<test_class, int, &test_class::compute_prop> ComputedProp;
    public_property<test_class, test_prop_type> ConvertPropSrc;

    void init_stringprop1()
        {
        StringProp1 = "Hello";
        }

    void init_stringprop2()
        {
        StringProp2 = "World";
        }

    test_class()
        : IntProp1(this)
        , IntProp2(this)
        , CharProp1(this)
        , StringProp1(this)
        , StringProp2(this)
        , ComputedProp(this)
        , ConvertPropSrc(this) { }
};


class test_class2 {
public:
    public_property<test_class2, int> IntProp1;
    public_property<test_class2, std::string> StringProp1;
    readonly_property<test_class2, std::string> ConvertPropTgt;

    void bind_convert_prop(
        test_class& t
        )
        {
        struct converter {
            std::string operator()(
                test_prop_type const& v
                )
                {
                return std::to_string(v.i);
                }
        };


        ConvertPropTgt.bind(t.ConvertPropSrc, converter{});
        }

    test_class2()
        : IntProp1(this)
        , StringProp1(this)
        , ConvertPropTgt(this) { }
};


class virtual_base {
    virtual bool set_prop(
        int&       o,
        int const& i
        )
        {
        o = i;
        return true;
        }

    virtual int get_prop() const
        {
        return 37;
        }

public:
    public_property<virtual_base, int, &virtual_base::set_prop> VirtProp {this};
    computed_property<virtual_base, int, &virtual_base::get_prop> VirtComputedProp {this};
};


class virtual_test
    : public virtual_base {
    bool set_prop(
        int&       o,
        int const& i
        ) override
        {
        o = i + 2;
        return true;
        }

    int get_prop() const override
        {
        return 42;
        }
};


TEST_CASE("Properties can be get and set") {
    test_class t;

    SECTION("The default setter has no validation") {
        t.IntProp1 = 5;
        REQUIRE(t.IntProp1 == 5);
        t.init_stringprop1();
        REQUIRE_THAT(t.StringProp1, Catch::Matchers::Equals("Hello"));
        }
    SECTION("Properties can have custom setters") {
        t.IntProp2 = 8;
        REQUIRE(t.IntProp2 == 8);
        t.IntProp2 = 12;
        REQUIRE(t.IntProp2 == 8);
        }
    SECTION("Properties can use a backing field") {
        t._backingChar   = 'g';
        REQUIRE(t.CharProp1 == 'g');
        t.CharProp1      = 'x';
        REQUIRE(t.CharProp1 == 'x');
        REQUIRE(t._backingChar == 'x');
        t._backingString = "Test";
        REQUIRE_THAT(t.StringProp2, Catch::Matchers::Equals("Test"));
        t.init_stringprop2();
        REQUIRE_THAT(t.StringProp2, Catch::Matchers::Equals("World"));
        REQUIRE_THAT(t._backingString, Catch::Matchers::Equals("World"));
        }
    }

TEST_CASE("Getters and setters can be virtual") {
    virtual_base b;
    virtual_test d;

    b.VirtProp = 7;
    REQUIRE(b.VirtProp == 7);
    d.VirtProp = 7;
    REQUIRE(d.VirtProp == 9);
    REQUIRE(b.VirtComputedProp == 37);
    REQUIRE(d.VirtComputedProp == 42);
    }

TEST_CASE("Some property type operators are forwarded") {
    test_class t;

    t.IntProp1 = 16;

    SECTION("Arithmetic operators") {
        REQUIRE(t.IntProp1 + 4 == 20);
        REQUIRE(t.IntProp1 - 7 == 9);
        REQUIRE(t.IntProp1 / 4 == 4);
        REQUIRE(t.IntProp1 * 2 == 32);
        REQUIRE(t.IntProp1 % 5 == 1);
        REQUIRE(-t.IntProp1 == -16);
        REQUIRE(~t.IntProp1 == -17);
        REQUIRE((t.IntProp1 | 4) == 20);
        REQUIRE((t.IntProp1 & 16) == 16);
        REQUIRE((t.IntProp1 ^ 16) == 0);
        REQUIRE((t.IntProp1 << 1) == 32);
        REQUIRE((t.IntProp1 >> 1) == 8);
        }
    SECTION("Assignment operators") {
        t.IntProp1  += 4;
        REQUIRE(t.IntProp1 == 20);
        t.IntProp1  -= 6;
        REQUIRE(t.IntProp1 == 14);
        t.IntProp1  /= 2;
        REQUIRE(t.IntProp1 == 7);
        t.IntProp1  *= 3;
        REQUIRE(t.IntProp1 == 21);
        t.IntProp1  %= 5;
        REQUIRE(t.IntProp1 == 1);
        t.IntProp1 <<= 3;
        REQUIRE(t.IntProp1 == 8);
        t.IntProp1 >>= 1;
        REQUIRE(t.IntProp1 == 4);
        t.IntProp1  |= 16;
        REQUIRE(t.IntProp1 == 20);
        t.IntProp1  &= 16;
        REQUIRE(t.IntProp1 == 16);
        t.IntProp1  ^= 20;
        REQUIRE(t.IntProp1 == 4);
        }
    }

TEST_CASE("Setting a property emits a Changed signal") {
    test_class t;
    auto       propValue = 0;

    t.IntProp2.Changed.connect([&](int const& v)
        {
        propValue = v;
        });
    t.IntProp2 = 16;
    REQUIRE(propValue == 0);
    t.IntProp2 = 3;
    REQUIRE(propValue == 3);
    }

TEST_CASE("Properties can be computed") {
    test_class t;

    t._computeAdd = 4;
    REQUIRE(t.ComputedProp == 7);
    }

TEST_CASE("Properties can be bound") {
    test_class  t1;
    test_class2 t2;

    SECTION("Implicitly convertible properties can be bound without a converter") {
        auto binding = t2.IntProp1.bind(t1.IntProp2);

        t1.IntProp2  = 6;
        REQUIRE(t2.IntProp1 == 6);
        binding->disconnect();
        t1.IntProp2  = 4;
        REQUIRE(t2.IntProp1 == 6);
        t2.IntProp1.bind(t1.CharProp1);
        t1.CharProp1 = 'e';
        REQUIRE(t2.IntProp1 == 101);
        t2.StringProp1.bind(t1.StringProp1);
        t1.init_stringprop1();
        REQUIRE_THAT(t2.StringProp1, Catch::Matchers::Equals("Hello"));
        }
    SECTION("Bindings can be two-way") {
        t2.IntProp1.bind(t1.IntProp1);
        t1.IntProp1.bind(t2.IntProp1);
        t2.IntProp1 = 37;
        REQUIRE(t1.IntProp1 == 37);
        t1.IntProp1 = 608;
        REQUIRE(t2.IntProp1 == 608);
        }
    SECTION("Inconvertible properties can use a custom converter") {
        t1.ConvertPropSrc = test_prop_type{42};
        t2.bind_convert_prop(t1);
        REQUIRE_THAT(t2.ConvertPropTgt, Catch::Matchers::Equals("42"));
        t1.ConvertPropSrc = test_prop_type{37};
        REQUIRE_THAT(t2.ConvertPropTgt, Catch::Matchers::Equals("37"));
        struct strconverter {
            int operator()(
                std::string const& s
                )
                {
                if (s.size() < 1) return 0;
                return s[0];
                }
        };


        t2.IntProp1.bind(t1.StringProp1, strconverter{});
        t1.init_stringprop1();
        REQUIRE(t2.IntProp1 == 72);
        }
    }
