using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Elysium
{
    public static class Debug
    {
        public static void Log(object message)
        {
            InternalCalls.Debug_Log(Format(message));
        }

        public static void LogWarning(object message)
        {
            InternalCalls.Debug_LogWarning(Format(message));
        }

        public static void LogError(object message)
        {
            InternalCalls.Debug_LogError(Format(message));
        }
        private static string Format(object message)
        {
            if (message == null)
            {
                return "null";
            }
            return message.ToString();
        }
    }
}
