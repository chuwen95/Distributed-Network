//
// Created by chu on 3/25/25.
//

#ifndef RANDOMNUMBER_H
#define RANDOMNUMBER_H

namespace csm
{

    namespace utilities
    {

        class RandomNumber
        {
        public:
            static int lcrc(int min, int max);

            static int loro(int min, int max);

            static int lorc(int min, int max);

            static int lcro(int min, int max);
        };

    }

}

#endif //RANDOMNUMBER_H
