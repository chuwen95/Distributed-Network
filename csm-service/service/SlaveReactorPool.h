//
// Created by chu on 11/14/25.
//

#ifndef COPYSTATEMACHINE_SLAVEREACTORPOOL_H
#define COPYSTATEMACHINE_SLAVEREACTORPOOL_H

#include "SlaveReactor.h"

namespace csm
{

    namespace service
    {

        class SlaveReactorPool
        {
            public:
            explicit SlaveReactorPool(std::size_t size);

        public:
            SlaveReactor* slaveReactor(std::size_t index);

            std::vector<SlaveReactor*> slaveReactors();

        private:
            std::vector<std::unique_ptr<SlaveReactor>> m_slaveReactors;
        };

    }

}

#endif //COPYSTATEMACHINE_SLAVEREACTORPOOL_H