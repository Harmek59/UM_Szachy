import chess
import ctypes


class ChessEngine:
    def __init__(self):
        self.engine_lib = ctypes.cdll.LoadLibrary('../ChessEngine/engineDll/ChessEngine.dll')
        self.run_function = self.engine_lib.run
        self.run_function.argtypes = [ctypes.c_char_p,
                                      ctypes.c_char_p]  # first is board fen, second is output buffer for move
        self.run_function.restype = ctypes.c_float  # return position evaluation

        self.move_char_buffer = ctypes.create_string_buffer(6)  # buffer to save best move to

    def run_engine(self, board: chess.Board):
        evaluation: float = self.run_function(board.fen().encode(), self.move_char_buffer)
        move_str: str = self.move_char_buffer.value.decode("utf-8")
        return [chess.Move.from_uci(move_str), evaluation]
