// ++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDLIB.H>
#include <STRING.H>

typedef int _Cmpfun(const void *, const void *);

#define MAX_BUF 256

void (qsort)(void *base, size_t n, size_t size, _Cmpfun *cmp)
{
    while (1 < n)
    {
        size_t i = 0;
        size_t j = n - 1;
        char *qi = base;
        char *qj = qi + size * j;
        char *qp = qj;

        while (i < j)
        {
            while (i < j && (*cmp)(qi, qp) <= 0)
                ++i, qi += size;

            while (i < j && (*cmp)(qp, qj) <= 0)
                --j, qj -= size;

            if (i < j)
            {
                char buf[MAX_BUF];
                char *q1 = qi;
                char *q2 = qj;
                size_t m, ms;

                for (ms = size; 0 < ms; ms -= m, q1 += m, q2 -= m)
                {
                    m = ms < sizeof(buf) ? ms : sizeof(buf);
                    memcpy(buf, q1, m);
                    memcpy(q1, q2, m);
                    memcpy(q2, buf, m);
                }
                ++i, qi += size;
                --j, qj -= size;
            }
        }
        if (qi != qp)
        {
                char buf[MAX_BUF];
                char *q1 = qi;
                char *q2 = qp;
                size_t m, ms;

                for (ms = size; 0 < ms; ms -= m, q1 += m, q2 -= m)
                {
                m = ms < sizeof(buf) ? ms : sizeof(buf);
                memcpy(buf, q1, m);
                memcpy(q1, q2, m);
                memcpy(q2, buf, m);
                }
        }
        j = n - i;

        if (j < i)
        {
                if (1 < j)
                qsort(qi, j, size, cmp);

                n = i;
        }
        else
        {
                if (1 < i)
                qsort(base, i, size, cmp);

                base = qi;
                n = j;
        }
    }
}
