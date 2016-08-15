#ifndef COORD_H
#define COORD_H

struct Coord2D {
    int x,y;
    Coord2D() {
        x=y=0;
    }
    Coord2D(int nx, int ny) {
        x=nx; y = ny;;
    }
    Coord2D(const Coord2D& c) {
        x = c.x; y = c.y;
    }
    Coord2D& operator = (const Coord2D &c) {
        x = c.x; y = c.y;;
        return *this;
    }
    bool operator < (const Coord2D &c) const {
        if ( y < c.y ) {
            return true;
        } else if ( y == c.y ) {
            return x < c.x;
        }
        return false;
    }
    bool operator <= (const Coord2D &c) const {
        if ( y < c.y ) {
            return true;
        } else if ( y == c.y ) {
            return x <= c.x;
        }
        return false;
    }
    bool operator == (const Coord2D &c) const {
        return (x==c.x && y==c.y);
    }
};
struct Coord3D {
    int x,y,z;
    Coord3D() {
        x=y=z=0;
    }
    Coord3D(int nx, int ny, int nz) {
        x=nx; y = ny; z = nz;
    }
    Coord3D(const Coord3D& c) {
        x = c.x; y = c.y; z = c.z;
    }
    Coord3D& operator = (const Coord3D &c) {
        x = c.x; y = c.y; z = c.z;
        return *this;
    }
    Coord3D& operator + (const Coord3D &c) {
        x += c.x; y += c.y; z += c.z;
        return *this;
    }
    Coord3D& operator += (const Coord3D &c) {
        x += c.x; y += c.y; z += c.z;
        return *this;
    }
    bool operator < (const Coord3D &c) const {
        if ( z < c.z ) {
            return true;
        } else if ( z == c.z ) {
            if ( y < c.y ) {
                return true;
            } else if ( y == c.y ) {
                return x < c.x;
            }
        }
        return false;
    }
    bool operator <= (const Coord3D &c) const {
        if ( z < c.z ) {
            return true;
        } else if ( z == c.z ) {
            if ( y < c.y )
            {
                return true;
            } else if ( y == c.y ) {
                return x <= c.x;
            }
        }
        return false;
    }
    bool operator == (const Coord3D &c) const {
        return (x==c.x && y==c.y && z==c.z);
    }
    bool operator != (const Coord3D &c) const {
        return (x!=c.x || y!=c.y || z!=c.z);
    }
        
};
struct CompareCoord2D {
    const bool operator()(const Coord2D& a, const Coord2D& b) const {
        return a < b;
    }
};
struct CompareCoord3D {
    const bool operator()(const Coord3D& a, const Coord3D& b) const {
        return a < b;
    }
};

#endif
