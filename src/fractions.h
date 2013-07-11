#ifndef FRACTIONS_H
#define FRACTIONS_H

#include <boost/scoped_array.hpp>
#include <exception>
#include <ostream>

/**
 * @brief A generic class to handle the fractions of types
 */
class Fractions
{
    private:
        boost::scoped_array<double> _data;
        bool                        _withCheck;
    protected:
        size_t                      _typeCount;
    public:

        /**
         * @brief Constructor
         *
         * @param typeCount The amount of types
         * @param withCheck Should the consistency be checked all the time?
         */
        Fractions (const size_t& typeCount, const bool withCheck = true);

        /**
         * @brief Copy constructor
         *
         * @param Fractions Reference to the object to be copied
         */
        Fractions (const Fractions&);

        /**
         * @brief 
         */
        void check () const;

        /**
         * @brief 
         *
         * @param index
         *
         * @return 
         */
        const double& operator[] (const size_t& index) const;

        /**
         * @brief 
         *
         * @param index
         * @param value
         */
        void set (const size_t& index, const double& value);

        /**
         * @brief 
         *
         * @param index
         * @param value
         */
        void add (const size_t& index, const double& value);

        /**
         * @brief 
         *
         * @return 
         */
        double missing () const;

        /**
         * @brief 
         *
         * @param out
         * @param fractions
         *
         * @return 
         */
        friend std::ostream& operator<< (std::ostream& out, const Fractions& fractions);
};

class FractionInconsistent : public std::exception {};
class FractionOutOfRange   : public std::exception {};

#endif
