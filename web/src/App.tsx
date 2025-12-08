import { useState, useRef, useEffect } from "react";
import { ArrowUp, ArrowDown, ArrowLeft, ArrowRight } from "lucide-react";

export default function App() {
  const ws = useRef<WebSocket | null>(null);
  const [connected, setConnected] = useState(false);
  const [isRecording, setIsRecording] = useState(false);
  const intervalRef = useRef<ReturnType<typeof setInterval> | null>(null);

  const connectWebSocket = () => {
    try {
      ws.current = new WebSocket("ws://172.20.10.2:80");

      ws.current.onopen = () => setConnected(true);
      ws.current.onclose = () => setConnected(false);
    } catch (error) {
      console.error("WebSocket connection failed:", error);
    }
  };

  const sendCommand = (cmd: string, record: boolean) => {
    if (ws.current && connected) {
      const message = JSON.stringify({
        command: cmd,
        record: record,
      });
      ws.current.send(message);
    }
  };

  const startHold = (cmd: string) => {
    sendCommand(cmd, isRecording);
    intervalRef.current = setInterval(() => sendCommand(cmd, isRecording), 150);
  };

  const stopHold = () => {
    if (intervalRef.current) clearInterval(intervalRef.current);
    intervalRef.current = null;
    sendCommand("STOP", isRecording);
  };

  const holdEvents = (cmd: string) => ({
    onMouseDown: () => startHold(cmd),
    onMouseUp: stopHold,
    onMouseLeave: stopHold,
    onTouchStart: () => startHold(cmd),
    onTouchEnd: stopHold,
    onTouchCancel: stopHold,
  });

  const handleRecordToggle = () => {
    setIsRecording(!isRecording);
  };

  useEffect(() => {
    connectWebSocket();
  }, []);

  return (
    <div className="flex flex-col items-center mt-10 uppercase">
      <p className="mt-4 text-xl font-bold">
        Status:{" "}
        <span className={connected ? "text-green-500" : "text-red-500"}>
          {connected ? "Connected" : "Not Connected"}
        </span>
      </p>

      {/* Joystick */}
      <div className="mt-10 flex flex-col items-center select-none">
        <button
          {...holdEvents("FORWARD")}
          className="w-24 h-24 bg-gray-200 rounded-lg hover:bg-gray-300 flex items-center justify-center transition-colors duration-300"
        >
          <ArrowUp size={40} />
        </button>

        <div className="flex">
          <button
            {...holdEvents("LEFT")}
            className="w-24 h-24 bg-gray-200 rounded-lg m-2 hover:bg-gray-300 flex items-center justify-center transition-colors duration-300"
          >
            <ArrowLeft size={40} />
          </button>

          <button
            {...holdEvents("RIGHT")}
            className="w-24 h-24 bg-gray-200 rounded-lg m-2 hover:bg-gray-300 flex items-center justify-center transition-colors duration-300"
          >
            <ArrowRight size={40} />
          </button>
        </div>

        <button
          {...holdEvents("REVERSE")}
          className="w-24 h-24 bg-gray-200 rounded-lg hover:bg-gray-300 flex items-center justify-center transition-colors duration-300"
        >
          <ArrowDown size={40} />
        </button>

        <div className="flex font-bold text-xl gap-2 mt-6">
          <button
            onClick={handleRecordToggle}
            className={`w-28 h-16 rounded-lg flex items-center justify-center transition-colors duration-300 ${isRecording
              ? "bg-red-500 hover:bg-red-600 text-white"
              : "bg-gray-200 hover:bg-gray-300 text-black"
              }`}
          >
            {isRecording ? "STOP" : "RECORD"}
          </button>

          <button
            {...holdEvents("PLAY")}
            className="w-28 h-16 bg-gray-200 rounded-lg hover:bg-gray-300 flex items-center justify-center transition-colors duration-300"
          >
            PLAY
          </button>
        </div>
      </div>
    </div>
  );
}