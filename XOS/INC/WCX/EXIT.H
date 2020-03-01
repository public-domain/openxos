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

// Define process termination codes

#define TC_EXIT    1	/* Normal exit */
#define TC_KILL    2	/* Process killed */
#define TC_PIPE    3	/* No input available from pipe */
#define TC_NOVECT  4	/* No vector setup for software interrupt */
#define TC_BDUSTK  5	/* Bad user stack address */
#define TC_NOMEM   6	/* No memory available */
#define TC_UNIMOP  7	/* Unimplemented operation */
#define TC_RUNP    8	/* RUN SVC complete */
#define TC_RUNC    9
#define TC_ILLMDC  10	/* Illegal mode change */
#define TC_SUBTSK  11	/* Error terminating DOS sub-task */
#define TC_MEMCRPD 12	/* DOS memory allocation data corrupted */

