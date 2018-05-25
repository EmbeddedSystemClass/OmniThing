#ifndef OMNI_DIGITALOUTPUTPIN_H
#define OMNI_DIGITALOUTPUTPIN_H

#include "OutputVoid.h"
#include "OutputBool.h"

namespace omni
{
    class DigitalOutputPin : public OutputVoid, public OutputBool
    {
        private:
            unsigned char m_nPin;
            bool m_bInvertLogic;
            bool m_bValue;

        protected:
            virtual void writePin(bool b) = 0;

        public:
            DigitalOutputPin(unsigned char pin, bool initalValue=false, bool invertLogic=false);

            virtual ~DigitalOutputPin();

            unsigned char getPin() const {return m_nPin;}
            bool isInverted() const {return m_bInvertLogic;}
            bool getValue() const {return m_bValue;}

            void writeVoid() final; 
            void writeBool(bool b) final;
    };
}

#endif

