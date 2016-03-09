#ifndef TYPE_MAGIC_H
#define TYPE_MAGIC_H

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

#endif /* TYPE_MAGIC_H */
