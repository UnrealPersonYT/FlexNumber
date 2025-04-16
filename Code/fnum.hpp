#pragma once
#include <iostream>
#include <sstream>
#include <initializer_list>
#include <cstring>
#include <string>
#include <vector>

#pragma warning(push)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unrefrenced Inline Function ///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4514)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sign Mismatch /////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4365)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Possible Data Loss ////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4244)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// QSpectre Mitigation ///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 5045)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Flex Number Structure /////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct fnum
{
public:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum(void) : bytes({0}){}
    fnum(const fnum& other) : bytes(other.bytes){}
    fnum(const std::initializer_list<unsigned char> initList) : bytes(initList){}
    fnum(unsigned long long value)
    {
        while(value > 0)
        {
            bytes.push_back(static_cast<unsigned char>(value & 0xFF));
            value >>= 8;
        }
        if(bytes.empty()) bytes.push_back(0);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Assignment ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum& operator=(const fnum& other)
    {
        if(this != &other){ bytes = other.bytes; }
        return *this;
    }
    fnum& operator=(const unsigned long long other){ return *this = fnum(other); }
    fnum& operator=(const std::initializer_list<unsigned char> initList){ return *this = fnum(initList); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Bitwise ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    // AND /////////////////////////////////////////////
    ////////////////////////////////////////////////////
    fnum& operator&=(const fnum& other)
    {
        unsigned long long minSize = std::min(bytes.size(), other.bytes.size());
        for(unsigned long long i = 0; i < minSize; ++i) bytes[i] &= other.bytes[i];
        bytes.resize(minSize);
        trim();
        return *this;
    }
    fnum operator&(const fnum& other) const{ return fnum(*this) &= other; }

    ////////////////////////////////////////////////////
    // OR //////////////////////////////////////////////
    ////////////////////////////////////////////////////
    fnum& operator|=(const fnum& other)
    {
        unsigned long long maxSize = std::max(bytes.size(), other.bytes.size());
        bytes.resize(maxSize, 0);
        for(unsigned long long i = 0; i < other.bytes.size(); ++i)
        {
            bytes[i] |= other.bytes[i];
        }
        return *this;
    }
    fnum operator|(const fnum& other) const { return fnum(*this) |= other; }

    ////////////////////////////////////////////////////
    // XOR /////////////////////////////////////////////
    ////////////////////////////////////////////////////
    fnum& operator^=(const fnum& other)
    {
        unsigned long long maxSize = std::max(bytes.size(), other.bytes.size());
        bytes.resize(maxSize, 0);
        for(unsigned long long i = 0; i < other.bytes.size(); ++i)
        {
            bytes[i] ^= other.bytes[i];
        }
        return *this;
    }
    fnum operator^(const fnum& other) const{ return fnum(*this) ^= other; }

    ////////////////////////////////////////////////////
    // NOT /////////////////////////////////////////////
    ////////////////////////////////////////////////////
    fnum operator~() const{
        fnum result(*this);
        for(auto& b : result.bytes)
        {
            b = ~b;
        }
        result.trim();
        return result;
    }

    ////////////////////////////////////////////////////
    // Left Shift //////////////////////////////////////
    ////////////////////////////////////////////////////
    fnum& operator<<=(unsigned int shift)
    {
        if(shift == 0) return *this;
        unsigned int byteShift = shift / 8;
        unsigned int bitShift = shift % 8;

        bytes.insert(bytes.begin(), byteShift, 0);

        if(bitShift > 0)
        {
            unsigned char carry = 0;
            for(unsigned long long i = 0; i < bytes.size(); ++i)
            {
                unsigned char newCarry = bytes[i] >> (8 - bitShift);
                bytes[i] = (bytes[i] << bitShift) | carry;
                carry = newCarry;
            }
            if(carry != 0) bytes.push_back(carry);
        }

        return *this;
    }
    fnum operator<<(unsigned int shift) const{ return fnum(*this) <<= shift; }

    ////////////////////////////////////////////////////
    // Right Shift /////////////////////////////////////
    ////////////////////////////////////////////////////
    fnum& operator>>=(unsigned int shift)
    {
        if(shift == 0) return *this;
        unsigned int byteShift = shift / 8;
        unsigned int bitShift = shift % 8;

        if(byteShift >= bytes.size())
        {
            bytes ={0};
            return *this;
        }

        bytes.erase(bytes.begin(), bytes.begin() + byteShift);

        if(bitShift > 0)
        {
            unsigned char carry = 0;
            for(int i = static_cast<int>(bytes.size()) - 1; i >= 0; --i)
            {
                unsigned char newCarry = bytes[i] << (8 - bitShift);
                bytes[i] = (bytes[i] >> bitShift) | carry;
                carry = newCarry;
            }
        }

        trim();
        return *this;
    }
    fnum operator>>(unsigned int shift) const{ return fnum (*this) >>= shift; }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Addition //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum& operator+=(const fnum& other)
    {
        unsigned long long maxSize = std::max(bytes.size(), other.bytes.size());
        bytes.resize(maxSize, 0);
        unsigned char carry = 0;

        for(unsigned long long i = 0; i < maxSize || carry; ++i)
        {
            if(i == bytes.size()) bytes.push_back(0);
            unsigned short sum = bytes[i] + (i < other.bytes.size() ? other.bytes[i] : 0) + carry;
            bytes[i] = static_cast<unsigned char>(sum & 0xFF);
            carry = static_cast<unsigned char>(sum >> 8);
        }

        return *this;
    }
    fnum& operator+=(const unsigned long long number){ return *this += fnum(number); }
    fnum operator+(const fnum& other) const{  return fnum(*this) += other; }
    fnum operator+(const unsigned long long number) const{ return fnum(*this) + fnum(number); }
    fnum& operator++(void){ return *this += 1; }
    fnum operator++(int){return ++fnum(*this); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Subtraction ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum& operator-=(const fnum& other)
    {
        if(*this < other) throw std::underflow_error("Negative result in unsigned subtraction");

        unsigned char borrow = 0;
        for(unsigned long long i = 0; i < bytes.size(); ++i)
        {
            int diff = bytes[i] - (i < other.bytes.size() ? other.bytes[i] : 0) - borrow;
            if(diff < 0)
            {
                diff += 256;
                borrow = 1;
            }else
            {
                borrow = 0;
            }
            bytes[i] = static_cast<unsigned char>(diff);
        }

        trim();
        return *this;
    }
    fnum& operator-=(const unsigned long long number){ return *this -= fnum(number); }
    fnum operator-(const fnum& other) const{ return fnum(*this) -= other; }
    fnum operator-(const unsigned long long number) const{ return fnum(*this) -= number; }
    fnum& operator--(void){ return *this -= 1; }
    fnum operator--(int){return --fnum(*this); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Boolean (Comparison) //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    // Pure Comparison /////////////////////////////////
    ////////////////////////////////////////////////////
    bool operator==(const fnum& other) const{ return bytes == other.bytes; }
    bool operator!=(const fnum& other) const{ return !(*this == other); }
    
    ////////////////////////////////////////////////////
    // Greater Than / Less Than ////////////////////////
    ////////////////////////////////////////////////////
    bool operator<(const fnum& other) const
    {
        if(bytes.size() != other.bytes.size()) return bytes.size() < other.bytes.size();
        for(unsigned long long i = bytes.size(); i-- > 0; ) if(bytes[i] != other.bytes[i]) return bytes[i] < other.bytes[i];
        return false;
    }
    bool operator>(const fnum& other) const{ return other < *this; }
    bool operator<=(const fnum& other) const{ return !(*this > other); }
    bool operator>=(const fnum& other) const{ return !(*this < other); }

    ////////////////////////////////////////////////////
    // Explicit ////////////////////////////////////////
    ////////////////////////////////////////////////////
    explicit operator bool(void) const
    {
        for(auto b : bytes) if(b != 0) return true;
        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Multiplication ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum& operator*=(const fnum& other) {
        std::vector<unsigned char> result(bytes.size() + other.bytes.size(), 0);

        for(unsigned long long i = 0; i < bytes.size(); ++i)
        {
            unsigned char carry = 0;
            for(unsigned long long j = 0; j < other.bytes.size() || carry; ++j)
            {
                unsigned int val = result[i + j] +
                                   bytes[i] * (j < other.bytes.size() ? other.bytes[j] : 0) +
                                   carry;
                result[i + j] = static_cast<unsigned char>(val & 0xFF);
                carry = static_cast<unsigned char>(val >> 8);
            }
        }

        bytes = std::move(result);
        trim();
        return *this;
    }
    fnum& operator*=(unsigned long long value){ return *this *= fnum(value); }
    fnum operator*(const fnum& other) const{ return fnum(*this) *= other; }
    fnum operator*(unsigned long long value) const{ return *this * fnum(value); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Division //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum& operator/=(const fnum& divisor)
    {
        if(divisor == 0) throw std::domain_error("Division by zero");
        if(*this < divisor) return *this = 0;
    
        fnum quotient;
        fnum remainder;
    
        for(int i = static_cast<int>(bytes.size() * 8) - 1; i >= 0; --i)
        {
            remainder <<= 1;
            if((bytes[i / 8] >> (i % 8)) & 1) remainder.bytes[0] |= 1;
            if(remainder >= divisor)
            {
                remainder -= divisor;
                if(quotient.bytes.size() <= static_cast<unsigned long long>(i / 8)) quotient.bytes.resize((i / 8) + 1, 0);
                quotient.bytes[i / 8] |= (1 << (i % 8));
            }
        }
        *this = quotient;
        trim();
        return *this;
    }
    fnum& operator/=(unsigned long long value){ return *this /= fnum(value);}
    fnum operator/(const fnum& other) const{return fnum(*this) /= other; }
    fnum operator/(unsigned long long value) const{ return *this / fnum(value); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Modulo ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    fnum& operator%=(const fnum& divisor)
    {
        if(divisor == 0) throw std::domain_error("Modulo by zero");
        if(*this < divisor) return *this;
    
        fnum remainder;
        fnum copy = *this;
        int totalBits = static_cast<int>(bytes.size() * 8);
    
        for(int i = totalBits - 1; i >= 0; --i)
        {
            remainder <<= 1;

            unsigned long long byteIndex = i / 8;
            unsigned long long bitIndex = i % 8;
    
            if(byteIndex < copy.bytes.size() && ((copy.bytes[byteIndex] >> bitIndex) & 1))
            {
                remainder.bytes[0] |= 1;
            }

            if(remainder >= divisor)
            {
                remainder -= divisor;
            }
        }
    
        *this = remainder;
        return *this;
    }   
    fnum& operator%=(unsigned long long value){ return *this %= fnum(value); }
    fnum operator%(const fnum& other) const{ return fnum(*this) %= other; }
    fnum operator%(unsigned long long value) const{ return *this % fnum(value); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // String Based //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string toString(void) const
    {
        std::string result = "0";
        std::string current = "1";

        for(unsigned char byte : bytes)
        {
            for(int bit = 0; bit < 8; ++bit)
            {
                if(byte & (1 << bit)) result = stringAddition(result, current);
                current = stringAddition(current, current);
            }
        }

        return result;
    }
    friend std::ostream& operator<<(std::ostream& os, const fnum& num){ return os << num.toString(); }

private:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Helpers ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void trim(void)
    {
        while(bytes.size() > 1 && bytes.back() == 0)
        {
            bytes.pop_back();
        }
    }

    std::string stringAddition(const std::string& a, const std::string& b) const
    {
        int carry = 0;
        int i = static_cast<int>(a.size()) - 1;
        int j = static_cast<int>(b.size()) - 1;
        std::string result;
        result.reserve(std::max(a.size(), b.size()) + 1);
    
        while (i >= 0 || j >= 0 || carry != 0)
        {
            int sum = carry;
            if(i >= 0)
            {
                sum += a[static_cast<unsigned long long>(i)] - '0';
                --i;
            }
            if(j >= 0)
            {
                sum += b[static_cast<unsigned long long>(j)] - '0';
                --j;
            }
            result.push_back(static_cast<char>((sum % 10) + '0'));
            carry = sum / 10;
        }
    
        std::reverse(result.begin(), result.end());
        return result;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Data //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<unsigned char> bytes;
};
#pragma warning(pop)