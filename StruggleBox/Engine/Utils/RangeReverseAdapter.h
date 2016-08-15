#ifndef RANGE_REVERSE_ADAPTER_H
#define RANGE_REVERSE_ADAPTER_H

template<typename Type> class reverse_adapter
{
    public:
    reverse_adapter(Type& c) : _container(c)
    { }
    
    typename Type::reverse_iterator begin()
    { return _container.rbegin(); }
    
    typename Type::reverse_iterator end()
    { return _container.rend(); }
private:
    Type& _container;
};

template<typename Type> class const_reverse_adapter
{
public:
    const_reverse_adapter(const Type& c) : _container(c)
    { }
    
    typename Type::reverse_iterator begin()
    { return _container.rbegin(); }
    
    typename Type::reverse_iterator end()
    { return _container.rend(); }
private:
    const Type& _container;
};

template<typename Type> reverse_adapter<Type> reverse_adapt_container(Type &c)
{
    return reverse_adapter<Type>(c);
}

template<typename Type> reverse_adapter<Type> reverse_adapt_container(const Type &c)
{
    return const_reverse_adapter<Type>(c);
}

#endif /* RANGE_REVERSE_ADAPTER_H */
