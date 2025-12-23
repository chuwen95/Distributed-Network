//
// Created by ChuWen on 9/5/23.
//

#ifndef TCPSERVER_RINGBUFFER_H
#define TCPSERVER_RINGBUFFER_H

#include "csm-common/Common.h"

namespace csm
{

    namespace utilities
    {

        class RingBuffer
        {
        public:
            using Ptr = std::shared_ptr<RingBuffer>;

            explicit RingBuffer(std::size_t size);
            ~RingBuffer() = default;

        public:
            /**
             * @brief   初始化环形缓冲区
             * @return
             */
            int init();

            /**
             * @brief   反初始化环形缓冲区
             * @return
             */
            int uninit();

            /**
             * @brief   写入数据到环形缓冲区
             *                  注意本接口是拷贝数据到环形缓冲区中
             * @param data
             * @param len
             * @return
             */
            int writeData(const char *data, const std::size_t len);

            /**
             * @brief   从缓冲区获取指定长度的数据，
             *                  如果该函数返回0，表述缓冲区中没有这么多数据
             *                  如果返回1，表示获取成功
             *                  此函数无论如何都会进行缓冲区数据拷贝
             * @param size
             * @param data
             * @return
             */
            int readData(const std::size_t size, const std::size_t offset, std::vector<char> &data);

            /**
             * @brief       直接返回缓冲区的指针
             * @return
             */
            char *data();

            /**
             * @brief       获取起始offset
             * @return
             */
            std::size_t startOffset();

            /**
             * @brief       获取结束offset
             * @return
             */
            std::size_t endOffset();

            /**
             * @brief       返回缓冲区大小
             * @return
             */
            std::size_t size();

            /**
             * @brief   当前缓冲区中有多少数据
             *
             * @return
             */
            std::size_t dataLength();

            /**
             * @brief   当前缓冲区中的空闲空间
             *
             * @return
             */
            std::size_t space();

            /**
             * @brief   清空缓冲区，注意此函数没有清空std::vector<char>中的内容，只是置空两个offset
             */
            void clear();

            /**
             * @brief 得到可供写入的指针和长度
             *              如果当前是startOffset > endOffset, 则buffer = data() + endOffset, length是后面的空间，
             *              如果startOffset = 0, 为了不让startOffset == endOffset, 所以length = size - endOffset - 2
             *
             * @param buffer
             * @param length
             * @return
             */
            int getBufferAndLengthForWrite(char *&buffer, std::size_t &length);

            /**
             * @brief 增加已经使用的空间
             *
             * @param size
             * @return
             */
            int increaseUsedSpace(const std::size_t size);

            /**
             * @brief 获取length长度的缓冲区
             *              注意这个函数是从m_startOffset开始往后获取，m_startOffset > m_endOffset的时候可能出现m_size - m_startOffset < length的情况，
             *              那就需要拷贝了，调用readData接口拷贝数据出来吧
             *
             * @param length
             * @param buffer
             * @return
             */
            int getBufferForRead(const std::size_t size, char *&buffer);

            /**
             * @brief  主要用于当数据被用掉后，降低usedSpace
             *
             * @param size
             * @return
             */
            int decreaseUsedSpace(const std::size_t size);

            /**
             * @brief 从缓冲区中获取连续的数据
             * 如果m_startOffset < m_endOffset，则buffer = data() + m_startOFfset, size = m_endOffset - m_startOffset
             * 如果m_endOffset > m_startOffset, 则buffer = data() + m_startOffset, size = m_size - m_startOffset
             *
             * @param buffer
             * @param size
             * @return
             */
            int getContinuousData(char *&buffer, std::size_t &size);

        private:
            std::size_t m_size{0};
            std::vector<char> m_buffer;
            /*
             * 实际缓冲区中的数据是[m_startOffset, m_endOffset)
             * 比如现在缓冲区是空的，m_startOffset和m_endOffset现在是都是0，写入8个数据，那么m_endOffset += 8
             * 但实际装在缓冲区中的数据是
             * 0 1 2 3 4 5 6 7
             * 所以8是终止位，也是继续写入的起始位，data() + m_endOffset即为后续写入的地址
             * 如果m_startOffset或m_endOffset等于了m_size，那么会赋值为0
             */
            std::size_t m_startOffset{0};
            std::size_t m_endOffset{0};
        };

    }

}

#endif //TCPSERVER_RINGBUFFER_H