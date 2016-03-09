#ifndef TypeMagic_h
#define TypeMagic_h

#include <memory>

class TypeMagic
{
public:
    template <typename T_>
    static int MagicNumberFor()
    {
        static int result(NextMagicNumber());
        return result;
    }
private:
    static int NextMagicNumber()
    {
        static int magic(0);
        return magic++;
    }
};

#endif /* TypeMagic_h */
