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
        boost::scoped_array<double> data_;
        size_t                      typeCount_;
    protected:
        inline const size_t& typeCount() const {return typeCount_;}
    public:

        /**
         * @brief Constructor
         *
         * @param typeCount The amount of types
         */
        Fractions (size_t typeCount);

        /**
         * @brief Copy constructor
         *
         * @param Fractions Reference to the object to be copied
         */
        Fractions (const Fractions&);

        /**
         * @brief 
         */
        void check() const;

        /**
         * @brief 
         *
         * @param index
         *
         * @return 
         */
        const double& operator[] (size_t index) const;

        /**
         * @brief 
         *
         * @param index
         * @param value
         */
        void set (size_t index, double value);

        /**
         * @brief 
         *
         * @param index
         * @param value
         */
        void add (size_t index, double value);

        /**
         * @brief 
         *
         * @return 
         */
        double missing() const;

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
