/*
 * Member property implementation suitable for use in UI
 * libraries, with change notifications and access control.
 * Author: mousebyte (ateague063@gmail.com)
 * Version: 1.0, Jan 2021
 * */

#ifndef MB_MEMPROP_HPP
#define MB_MEMPROP_HPP
#include <type_traits>
#include <sigslot/signal.hpp>

namespace mousebyte {
    namespace memprop {
        /**
         * @brief Provides access to a property binding.
         */
        class binding {
        protected:
            sigslot::connection _connection;
            binding() = default;

        public:
            binding(binding const&)            = delete;
            binding(binding&&)                 = delete;
            binding& operator=(binding const&) = delete;
            binding& operator=(binding&&)      = delete;

            /**
             * @brief Checks if the binding is active.
             *
             * @return True if the binding is active, false
             * if the binding has been disconnected.
             */
            bool active() const
                {
                return _connection.connected();
                }

            /**
             * @brief Disconnects the binding.
             */
            void disconnect()
                {
                _connection.disconnect();
                }

            virtual ~binding()
                {
                disconnect();
                }
        };


        namespace detail {
            //member function helper aliases

            template <typename Owner, typename T>
            using mem_getter = T (Owner::*)() const;

            template <typename Owner, typename T>
            using mem_setter = bool (Owner::*)(T&, T const&);

            template <typename Owner, typename T>
            using mem_setter_backed = bool (Owner::*)(T const&);

            template <typename, typename, typename>
            class binding_impl;
            template <typename>
            class core_binding_access;
            }

        template <typename Owner, typename V, detail::mem_getter<Owner, V> Get>
        class computed_property;
        template <typename Owner, typename V,
                  detail::mem_setter<Owner, std::remove_cvref_t<V>> Set>
        class public_property;
        template <typename Owner, typename V,
                  detail::mem_getter<Owner, std::remove_cvref_t<V> const&> Get,
                  detail::mem_setter_backed<Owner, std::remove_cvref_t<V>> Set>
        class backed_public_property;
        template <typename Owner, typename V,
                  detail::mem_setter<Owner, std::remove_cvref_t<V>> Set>
        class readonly_property;
        template <typename Owner, typename V,
                  detail::mem_getter<Owner, std::remove_cvref_t<V> const&> Get,
                  detail::mem_setter_backed<Owner, std::remove_cvref_t<V>> Set>
        class backed_readonly_property;

        namespace detail {
            //operator support concepts

            template <typename T1, typename T2>
            concept Addable              = requires(T1 t1, T2 t2) {
                t1 + t2;
                };
            template <typename T1, typename T2>
            concept Subtractable         = requires(T1 t1, T2 t2) {
                t1 - t2;
                };
            template <typename T1, typename T2>
            concept Multipliable         = requires(T1 t1, T2 t2) {
                t1 * t2;
                };
            template <typename T1, typename T2>
            concept Divisible            = requires(T1 t1, T2 t2) {
                t1 / t2;
                };
            template <typename T1, typename T2>
            concept HasModulo            = requires(T1 t1, T2 t2) {
                t1 % t2;
                };
            template <typename T>
            concept HasBitwiseNot        = requires(T t) {
                ~t;
                };
            template <typename T1, typename T2>
            concept HasBitwiseAnd        = requires(T1 t1, T2 t2) {
                t1 & t2;
                };
            template <typename T1, typename T2>
            concept HasBitwiseOr         = requires(T1 t1, T2 t2) {
                t1 | t2;
                };
            template <typename T1, typename T2>
            concept HasBitwiseXor        = requires(T1 t1, T2 t2) {
                t1 ^ t2;
                };
            template <typename T1, typename T2>
            concept HasLeftShift         = requires(T1 t1, T2 t2) {
                t1 << t2;
                };
            template <typename T1, typename T2>
            concept HasRightShift        = requires(T1 t1, T2 t2) {
                t1 >> t2;
                };
            template <typename T>
            concept PreIncrementable     = requires(T t) {
                ++t;
                };
            template <typename T>
            concept PostIncrementable    = requires(T t) {
                t++;
                };
            template <typename T>
            concept PreDecrementable     = requires(T t) {
                --t;
                };
            template <typename T>
            concept PostDecrementable    = requires(T t) {
                t--;
                };

            template <typename T1, typename T2>
            concept HasOperatorEq        = requires(T1 t1, T2 t2) {
                t1 == t2;
                };
            template <typename T1, typename T2>
            concept HasOperatorNotEq     = requires(T1 t1, T2 t2) {
                t1 == t2;
                };
            template <typename T1, typename T2>
            concept HasOperatorLess      = requires(T1 t1, T2 t2) {
                t1 < t2;
                };
            template <typename T1, typename T2>
            concept HasOperatorGreater   = requires(T1 t1, T2 t2) {
                t1 > t2;
                };
            template <typename T1, typename T2>
            concept HasOperatorLessEq    = requires(T1 t1, T2 t2) {
                t1 <= t2;
                };
            template <typename T1, typename T2>
            concept HasOperatorGreaterEq = requires(T1 t1, T2 t2) {
                t1 >= t2;
                };
            template <typename T1, typename T2>
            concept HasThreeWayCompare   = requires(T1 t1, T2 t2) {
                t1 <=> t2;
                };
            template <typename T>
            concept HasOperatorNot       = requires(T t) {
                !t;
                };
            template <typename T>
            concept Negatable            = requires(T t) {
                -t;
                };


            template <typename>
            struct property_traits {};


            template <typename Owner, typename V, detail::mem_getter<Owner,
                                                                     V> Get>
            struct property_traits<computed_property<Owner, V, Get>> {
                using owner_type      = Owner;
                using property_type   = computed_property<Owner, V, Get>;
                using value_type      = V;
                using const_reference = V;
            };


            template <typename Owner, typename V,
                      detail::mem_setter<Owner, std::remove_cvref_t<V>> Set>
            struct property_traits<public_property<Owner, V, Set>> {
                using owner_type      = Owner;
                using property_type   = public_property<Owner, V, Set>;
                using value_type      = std::remove_cvref_t<V>;
                using const_reference = value_type const&;
            };


            template <typename Owner, typename V>
            struct property_traits<public_property<Owner, V, nullptr>> {
                using owner_type      = Owner;
                using property_type   = public_property<Owner, V, nullptr>;
                using value_type      = std::remove_cvref_t<V>;
                using const_reference = value_type const&;
            };


            template <typename Owner, typename V,
                      detail::mem_setter<Owner, std::remove_cvref_t<V>> Set>
            struct property_traits<readonly_property<Owner, V, Set>> {
                using owner_type      = Owner;
                using property_type   = readonly_property<Owner, V, Set>;
                using value_type      = std::remove_cvref_t<V>;
                using const_reference = value_type const&;
            };


            template <typename Owner, typename V>
            struct property_traits<readonly_property<Owner, V, nullptr>> {
                using owner_type      = Owner;
                using property_type   = readonly_property<Owner, V, nullptr>;
                using value_type      = std::remove_cvref_t<V>;
                using const_reference = value_type const&;
            };


            template <typename Owner, typename V,
                      detail::mem_getter<Owner, std::remove_cvref_t<V> const&> Get,
                      detail::mem_setter_backed<Owner, std::remove_cvref_t<V>> Set>
            struct property_traits<backed_public_property<Owner, V, Get, Set>> {
                using owner_type      = Owner;
                using property_type   = backed_public_property<Owner, V, Get, Set>;
                using value_type      = std::remove_cvref_t<V>;
                using const_reference = value_type const&;
            };


            template <typename Owner, typename V,
                      detail::mem_getter<Owner, std::remove_cvref_t<V> const&> Get,
                      detail::mem_setter_backed<Owner, std::remove_cvref_t<V>> Set>
            struct property_traits<backed_readonly_property<Owner, V, Get, Set>> {
                using owner_type      = Owner;
                using property_type   = backed_readonly_property<Owner, V, Get, Set>;
                using value_type      = std::remove_cvref_t<V>;
                using const_reference = value_type const&;
            };


            namespace traits {
                template <typename P>
                using owner_type      =
                    typename property_traits<P>::owner_type;
                template <typename P>
                using property_type   =
                    typename property_traits<P>::property_type;
                template <typename P>
                using value_type      =
                    typename property_traits<P>::value_type;
                template <typename P>
                using const_reference =
                    typename property_traits<P>::const_reference;
                }

            class dummy_converter { };


            //binding concepts

            template <typename PSrc, typename PTarget>
            concept PropertyConvertible = std::convertible_to<
                traits::const_reference<PSrc>, traits::const_reference<PTarget>>;
            template <typename PSrc, typename PTarget, typename Converter>
            concept ValidConverter      = requires(traits::const_reference<PSrc> v, Converter c) {
                    { c(v) }->std::convertible_to<traits::const_reference<PTarget>>;
                };

            template <typename Prop>
            class gettable_prop {
                using owner_type      = detail::traits::owner_type<Prop>;
                owner_type* _owner;

            protected:
                using const_reference = detail::traits::const_reference<Prop>;

                template <auto Pmf, typename ... Args>
                requires requires(
                    owner_type* p,
                    Args&&...   args
                    ) { ((*p).*Pmf)(std::forward<Args>(args)...); }
                inline std::result_of_t<decltype(Pmf)(owner_type, Args&&...)>
                call_owner_fn(
                    Args&&... args
                    )
                    {
                    return ((*_owner).*Pmf)(std::forward<Args>(args)...);
                    }

                template <auto Pmf, typename ... Args>
                requires requires(
                    owner_type const* p,
                    Args&&...         args
                    ) { ((*p).*Pmf)(std::forward<Args>(args)...); }
                inline std::result_of_t<decltype(Pmf)(const owner_type, Args&&...)>
                call_owner_fn(
                    Args&&... args
                    ) const
                    {
                    return ((*_owner).*Pmf)(std::forward<Args>(args)...);
                    }

                virtual const_reference get() const = 0;

                gettable_prop(
                    owner_type* owner
                    )
                    : _owner(owner)
                    {
                    }

            public:
                gettable_prop(gettable_prop const&)            = delete;
                gettable_prop(gettable_prop&&)                 = delete;
                gettable_prop& operator=(gettable_prop const&) = delete;
                gettable_prop& operator=(gettable_prop&&)      = delete;

                virtual ~gettable_prop() = default;

                template <typename T>
                requires detail::Addable<const_reference, T const&>
                friend auto operator+(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() + rhs;
                    }

                template <typename T>
                requires detail::Subtractable<const_reference, T const&>
                friend auto operator-(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() - rhs;
                    }

                template <typename T>
                requires detail::Multipliable<const_reference, T const&>
                friend auto operator*(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() * rhs;
                    }

                template <typename T>
                requires detail::Divisible<const_reference, T const&>
                friend auto operator/(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() / rhs;
                    }

                template <typename T>
                requires detail::HasModulo<const_reference, T const&>
                friend auto operator%(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() % rhs;
                    }

                template <typename T>
                requires detail::HasOperatorEq<const_reference, T const&>
                friend auto operator==(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() == rhs;
                    }

                template <typename T>
                requires detail::HasOperatorNotEq<const_reference, T const&>
                friend auto operator!=(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() != rhs;
                    }

                template <typename T>
                requires detail::HasOperatorLess<const_reference, T const&>
                friend auto operator<(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() < rhs;
                    }

                template <typename T>
                requires detail::HasOperatorGreater<const_reference, T const&>
                friend auto operator>(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() > rhs;
                    }

                template <typename T>
                requires detail::HasOperatorGreaterEq<const_reference, T const&>
                friend auto operator>=(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() >= rhs;
                    }

                template <typename T>
                requires detail::HasOperatorLessEq<const_reference, T const&>
                friend auto operator<=(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() <= rhs;
                    }

                template <typename T>
                requires detail::HasThreeWayCompare<const_reference, T const&>
                friend auto operator<=>(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() <=> rhs;
                    }

                friend auto operator~(
                    gettable_prop<Prop> const& v
                    ) requires detail::HasBitwiseNot<const_reference>
                    {
                    return ~v.get();
                    }

                template <typename T>
                requires detail::HasBitwiseAnd<const_reference, T const&>
                friend auto operator&(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() & rhs;
                    }

                template <typename T>
                requires detail::HasBitwiseOr<const_reference, T const&>
                friend auto operator|(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() | rhs;
                    }

                template <typename T>
                requires detail::HasBitwiseXor<const_reference, T const&>
                friend auto operator^(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() ^ rhs;
                    }

                template <typename T>
                requires detail::HasLeftShift<const_reference, T const&>
                friend auto operator<<(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() << rhs;
                    }

                template <typename T>
                requires detail::HasRightShift<const_reference, T const&>
                friend auto operator>>(
                    gettable_prop<Prop> const& lhs,
                    T const&                   rhs
                    )
                    {
                    return lhs.get() >> rhs;
                    }

                friend auto operator!(
                    gettable_prop<Prop> const& v
                    ) requires detail::HasOperatorNot<const_reference>
                    {
                    return !v.get();
                    }

                friend auto operator-(
                    gettable_prop<Prop> const& v
                    ) requires detail::Negatable<const_reference>
                    {
                    return -v.get();
                    }
            };


            template <typename Prop>
            class settable_prop
                : public gettable_prop<Prop> {
            protected:
                template <typename, typename, typename>
                friend class binding_impl;

                using const_reference = detail::traits::const_reference<Prop>;

                settable_prop(
                    detail::traits::owner_type<Prop>* owner
                    )
                    : gettable_prop<Prop>(owner)
                    {
                    }

                void invoke_changed(
                    const_reference v
                    )
                    {
                    Changed(v);
                    }

                bool set(
                    const_reference v
                    )
                    {
                    auto success = do_set(v);

                    if (success) {
                        invoke_changed(this->get());
                        }
                    return success;
                    }

            public:
                sigslot::signal_ix<settable_prop<Prop>, const_reference> Changed;

            private:
                virtual bool do_set(const_reference) = 0;
            };


            template <typename PSrc, typename PTarget, typename Converter = detail::dummy_converter>
            class binding_impl
                : public binding {
                settable_prop<PSrc>* _source;
                settable_prop<PTarget>* _target;
                Converter _converter;

                template <typename>
                friend class core_binding_access;
                void set_target_value(
                    detail::traits::const_reference<PSrc> v
                    )
                    {
                    if constexpr (!std::same_as<Converter, detail::dummy_converter>) {
                        _target->set(_converter(v));
                        } else {
                        _target->set(v);
                        }
                    }

                void on_changed(
                    detail::traits::const_reference<PSrc> v
                    )
                    {
                    _connection.block();
                    set_target_value(v);
                    _connection.unblock();
                    }

                void init()
                    {
                    set_target_value(_source->get());
                    _connection = _source->Changed
                                  .connect(&binding_impl<PSrc, PTarget, Converter>::on_changed,
                                           this);
                    }

                binding_impl(
                    settable_prop<PSrc>*    src,
                    settable_prop<PTarget>* target
                    )
                    : _source(src)
                    , _target(target)
                    {
                    init();
                    }

                binding_impl(
                    settable_prop<PSrc>*    src,
                    settable_prop<PTarget>* target,
                    Converter&&             converter
                    )
                    : _source(src)
                    , _target(target)
                    , _converter(std::forward<Converter>(converter))
                    {
                    init();
                    }
            };


            template <typename Prop>
            class core_binding_access
                : public settable_prop<Prop> {
                std::shared_ptr<binding> _binding;

            protected:
                core_binding_access(
                    detail::traits::owner_type<Prop>* owner
                    )
                    : settable_prop<Prop>(owner)
                    {
                    }

                template <typename PSrc>
                std::shared_ptr<binding> bind_internal(
                    settable_prop<PSrc>* src
                    )
                    {
                    reset_binding();
                    using binding_t = binding_impl<PSrc, Prop>;
                    _binding = std::shared_ptr<binding_t>(new binding_t(src, this));
                    return _binding;
                    }

                template <typename PSrc, typename Converter>
                std::shared_ptr<binding> bind_internal(
                    settable_prop<PSrc>* src,
                    Converter&&          converter
                    )
                    {
                    reset_binding();
                    using binding_t = binding_impl<PSrc, Prop, Converter>;
                    _binding = std::shared_ptr<binding_t>(
                        new binding_t(src, this, std::forward<Converter>(converter)));
                    return _binding;
                    }

                void reset_binding()
                    {
                    if (_binding) {
                        _binding->disconnect();
                        _binding.reset();
                        }
                    }
            };
            } // namespace detail


        /**
         * @brief Exposes a property whose value is computed each time it is accessed.
         *
         * @tparam Owner The type that contains the property.
         * @tparam V The value type.
         * @tparam Get A pointer to the member function of Owner that gets the value.
         */
        template <typename Owner, typename V, detail::mem_getter<Owner, V> Get>
        class computed_property
            : public detail::gettable_prop<computed_property<Owner, V, Get>> {
        protected:
            using my_type = computed_property<Owner, V, Get>;
            using const_reference = detail::traits::const_reference<my_type>;
            const_reference get() const override
                {
                return this->template call_owner_fn<Get>();
                }

        public:
            computed_property(
                Owner* owner
                )
                : detail::gettable_prop<my_type>(owner)
                {
                }

            operator const_reference() const
                {
                return get();
                }
        };


        template <typename Prop>
        class public_property_base
            : public detail::core_binding_access<Prop> {
            using my_type = Prop;
        protected:
            public_property_base(
                detail::traits::owner_type<Prop>* owner
                )
                : detail::core_binding_access<Prop>(owner)
                {
                }

        public:
            using value_type      = detail::traits::value_type<my_type>;
            using const_reference = detail::traits::const_reference<my_type>;
            operator const_reference() const
                {
                return this->get();
                }

            /**
             * @brief Removes the binding from this property, if one exists.
             */
            void unbind()
                {
                this->reset_binding();
                }

            /**
             * @brief Binds this property to the value of another property.
             *
             * @param src The source property.
             *
             * @return A handle to the binding.
             */
            template <typename PSrc>
            requires detail::PropertyConvertible<PSrc, my_type>
            std::shared_ptr<binding> bind(
                detail::settable_prop<PSrc>& src
                )
                {
                return this->bind_internal(&src);
                }

            /**
             * @brief Binds this property to the value of another property
             * using the given converter object.
             *
             * @param src The source property.
             * @param converter The converter object. Must be a functor that accepts
             * a const reference to the source's value type and returns the target's
             * value type.
             *
             * @return A handle to the binding.
             */
            template <typename PSrc, typename Converter>
            requires detail::ValidConverter<PSrc, my_type, Converter>
            std::shared_ptr<binding> bind(
                detail::settable_prop<PSrc>& src,
                Converter&&                  converter
                )
                {
                return this->bind_internal(&src, std::forward<Converter>(converter));
                }

            template <typename T>
            requires detail::Addable<const_reference, T const&>
            friend auto& operator+=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() + rhs);
                return lhs;
                }

            template <typename T>
            requires detail::Subtractable<const_reference, T const&>
            friend auto& operator-=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() - rhs);
                return lhs;
                }

            template <typename T>
            requires detail::Multipliable<const_reference, T const&>
            friend auto& operator*=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() * rhs);
                return lhs;
                }

            template <typename T>
            requires detail::Divisible<const_reference, T const&>
            friend auto& operator/=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() / rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasModulo<const_reference, T const&>
            friend auto& operator%=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() % rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasBitwiseAnd<const_reference, T const&>
            friend auto& operator&=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() & rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasBitwiseOr<const_reference, T const&>
            friend auto& operator|=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() | rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasBitwiseXor<const_reference, T const&>
            friend auto& operator^=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() ^ rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasLeftShift<const_reference, T const&>
            friend auto& operator<<=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() << rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasRightShift<const_reference, T const&>
            friend auto& operator>>=(
                public_property_base<Prop>& lhs,
                T const&                    rhs
                )
                {
                lhs.set(lhs.get() >> rhs);
                return lhs;
                }
        };


        /**
         * @brief Exposes a property with a public getter and setter.
         *
         * @tparam Owner The type that contains the property.
         * @tparam V The value type.
         * @tparam Set A pointer to the member function of Owner to use as a setter.
         */
        template <typename Owner, typename V,
                  detail::mem_setter<Owner, std::remove_cvref_t<V>> Set = nullptr>
        class public_property
            : public public_property_base<public_property<Owner, V, Set>> {
            using my_type         = public_property<Owner, V, Set>;

            friend Owner;
        public:
            using value_type      = detail::traits::value_type<my_type>;
            using const_reference = detail::traits::const_reference<my_type>;
            public_property(
                Owner* owner
                )
                : public_property_base<my_type>(owner)
                {
                }

            public_property(
                Owner*          owner,
                const_reference v
                )
                : public_property_base<my_type>(owner)
                , _value(v)
                {
                }

            my_type& operator=(
                const_reference rhs
                )
                {
                this->set(rhs);
                return *this;
                }

            value_type* operator->()
                {
                return &_value;
                }

        protected:
            const_reference get() const override
                {
                return _value;
                }

        private:
            bool do_set(
                const_reference v
                ) override
                {
                if constexpr (Set == nullptr) {
                    _value = v;
                    return true;
                    } else {
                    return this->template call_owner_fn<Set>(_value, v);
                    }
                }

            value_type _value;
        };


        /**
         * @brief Exposes a property with a public getter and setter that
         * uses a backing field.
         *
         * @tparam Owner The type that contains the property.
         * @tparam V The value type.
         * @tparam Get A pointer to the member function of Owner that gets the value.
         * @tparam Set A pointer to the member function of Owner that sets the value.
         */
        template <typename Owner, typename V,
                  detail::mem_getter<Owner, std::remove_cvref_t<V> const&> Get,
                  detail::mem_setter_backed<Owner, std::remove_cvref_t<V>> Set>
        class backed_public_property
            : public public_property_base<backed_public_property<Owner, V, Get, Set>> {
            using my_type         = backed_public_property<Owner, V, Get, Set>;
            friend Owner;
        public:
            using value_type      = detail::traits::value_type<my_type>;
            using const_reference = detail::traits::const_reference<my_type>;
            backed_public_property(
                Owner* owner
                )
                : public_property_base<my_type>(owner)
                {
                }

            my_type& operator=(
                const_reference rhs
                )
                {
                this->set(rhs);
                return *this;
                }

        protected:
            const_reference get() const override
                {
                return this->template call_owner_fn<Get>();
                }

        private:
            bool do_set(
                const_reference v
                ) override
                {
                return this->template call_owner_fn<Set>(v);
                }
        };


        template <typename Prop>
        class readonly_property_base
            : public detail::core_binding_access<Prop> {
            using my_type = detail::traits::property_type<Prop>;
        public:
            using const_reference = detail::traits::const_reference<my_type>;
            operator const_reference() const
                {
                return this->get();
                }

        protected:
            readonly_property_base(
                detail::traits::owner_type<Prop>* owner
                )
                : detail::core_binding_access<Prop>(owner)
                {
                }

            /**
             * @brief Removes the binding from this property, if one exists.
             */
            void unbind()
                {
                this->reset_binding();
                }

            /**
             * @brief Binds this property to the value of another property.
             *
             * @param src The source property.
             *
             * @return A handle to the binding.
             */
            template <typename PSrc>
            requires detail::PropertyConvertible<PSrc, my_type>
            std::shared_ptr<binding> bind(
                detail::settable_prop<PSrc>& src
                )
                {
                return this->bind_internal(&src);
                }

            /**
             * @brief Binds this property to the value of another property
             * using the given converter object.
             *
             * @param src The source property.
             * @param converter The converter object. Must be a functor that accepts
             * a const reference to the source's value type and returns the target's
             * value type.
             *
             * @return A handle to the binding.
             */
            template <typename PSrc, typename Converter>
            requires detail::ValidConverter<PSrc, my_type, Converter>
            std::shared_ptr<binding> bind(
                detail::settable_prop<PSrc>& src,
                Converter&&                  converter
                )
                {
                return this->bind_internal(&src, std::forward<Converter>(converter));
                }

            template <typename T>
            requires detail::Multipliable<const_reference, T const&>
            friend auto& operator*=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() * rhs);
                return lhs;
                }

            template <typename T>
            requires detail::Divisible<const_reference, T const&>
            friend auto& operator/=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() / rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasModulo<const_reference, T const&>
            friend auto& operator%=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() % rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasBitwiseAnd<const_reference, T const&>
            friend auto& operator&=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() & rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasBitwiseOr<const_reference, T const&>
            friend auto& operator|=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() | rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasBitwiseXor<const_reference, T const&>
            friend auto& operator^=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() ^ rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasLeftShift<const_reference, T const&>
            friend auto& operator<<=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() << rhs);
                return lhs;
                }

            template <typename T>
            requires detail::HasRightShift<const_reference, T const&>
            friend auto& operator>>=(
                readonly_property_base<Prop>& lhs,
                T const&                      rhs
                )
                {
                lhs.set(lhs.get() >> rhs);
                return lhs;
                }
        };


        /**
         * @brief Exposes a property with a public getter and a setter accessible only
         * by Owner.
         *
         * @tparam Owner The type that contains the property.
         * @tparam V The value type.
         * @tparam Set A pointer to the member function of Owner to use as a setter.
         */
        template <typename Owner, typename V,
                  detail::mem_setter<Owner, std::remove_cvref_t<V>> Set = nullptr>
        class readonly_property
            : public readonly_property_base<readonly_property<Owner, V, Set>> {
            using my_type         = readonly_property<Owner, V, Set>;

            friend Owner;
        public:
            using value_type      = detail::traits::value_type<my_type>;
            using const_reference = detail::traits::const_reference<my_type>;
            readonly_property(
                Owner* owner
                )
                : readonly_property_base<my_type>(owner)
                {
                }

            readonly_property(
                Owner*          owner,
                const_reference v
                )
                : readonly_property_base<my_type>(owner)
                , _value(v)
                {
                }

        protected:
            const_reference get() const override
                {
                return _value;
                }

            my_type& operator=(
                const_reference rhs
                )
                {
                this->set(rhs);
                return *this;
                }

            value_type const* operator->() const
                {
                return &_value;
                }

        private:
            bool do_set(
                const_reference v
                ) override
                {
                if constexpr (Set == nullptr) {
                    _value = v;
                    return true;
                    } else {
                    return this->template call_owner_fn<Set>(_value, v);
                    }
                }

            value_type _value;
        };


        /**
         * @brief Exposes a property with a public getter and a setter accessible only
         * by Owner. Uses a backing field.
         *
         * @tparam Owner The type that contains the property.
         * @tparam V The value type.
         * @tparam Get A pointer to the member function of Owner that gets the value.
         * @tparam Set A pointer to the member function of Owner that sets the value.
         */
        template <typename Owner, typename V,
                  detail::mem_getter<Owner, std::remove_cvref_t<V> const&> Get,
                  detail::mem_setter_backed<Owner, std::remove_cvref_t<V>> Set>
        class backed_readonly_property
            : public readonly_property_base<backed_readonly_property<Owner, V, Get, Set>> {
            using my_type         = backed_readonly_property<Owner, V, Get, Set>;
            friend Owner;
        public:
            using value_type      = detail::traits::value_type<my_type>;
            using const_reference = detail::traits::const_reference<my_type>;
            backed_readonly_property(
                Owner* owner
                )
                : readonly_property_base<my_type>(owner)
                {
                }

        protected:
            const_reference get() const override
                {
                return this->template call_owner_fn<Get>();
                }

            my_type& operator=(
                const_reference rhs
                )
                {
                this->set(rhs);
                return *this;
                }

        private:
            bool do_set(
                const_reference v
                ) override
                {
                return this->template call_owner_fn<Set>(v);
                }
        };
        }
    }
#endif
