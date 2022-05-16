import chess
import time


class AIModel:
    MaximizingPlayer = chess.WHITE

    def __init__(self):
        self.current_player = None
        self.current_best_action = None
        self.current_max_depth = 5

    def evaluate(self, board: chess.Board):
        if board.is_game_over():
            if board.result() == "1-0":
                return 1
            elif board.result() == "0-1":
                return 0
            else:
                return 0.5
        else:
            # return 0.5
            starting_board_pieces_value = 39
            val = 0
            for piece_type in chess.PIECE_TYPES:
                positions = board.pieces(piece_type, chess.WHITE)
                if piece_type == chess.PIECE_TYPES[0]:
                    val += len(positions)
                elif piece_type == chess.PIECE_TYPES[1]:
                    val += 3 * len(positions)
                elif piece_type == chess.PIECE_TYPES[2]:
                    val += 3 * len(positions)
                elif piece_type == chess.PIECE_TYPES[3]:
                    val += 5 * len(positions)
                elif piece_type == chess.PIECE_TYPES[4]:
                    val += 9 * len(positions)

            for piece_type in chess.PIECE_TYPES:
                positions = board.pieces(piece_type, chess.BLACK)
                if piece_type == chess.PIECE_TYPES[0]:
                    val -= len(positions)
                elif piece_type == chess.PIECE_TYPES[1]:
                    val -= 3 * len(positions)
                elif piece_type == chess.PIECE_TYPES[2]:
                    val -= 3 * len(positions)
                elif piece_type == chess.PIECE_TYPES[3]:
                    val -= 5 * len(positions)
                elif piece_type == chess.PIECE_TYPES[4]:
                    val -= 9 * len(positions)

            val /= starting_board_pieces_value


            return val

    def min_max(self, board: chess.Board, depth: int, alpha: float = -22222, beta: float = 22222):
        player = board.turn
        if board.is_game_over() or depth == self.current_max_depth:
            return self.evaluate(board)

        if player == self.MaximizingPlayer:
            val: float = -1
            for move in board.legal_moves:
                board_after_move = board.copy()
                board_after_move.push(move)

                new_val: float = self.min_max(board_after_move, depth + 1, alpha, beta)

                if new_val > val:
                    val = new_val

                if new_val >= beta:
                    return val

                if new_val > alpha:
                    alpha = new_val
                    if depth == 0:
                        self.current_best_action = move

            return val
        else:
            val: float = 2
            for move in board.legal_moves:
                board_after_move = board.copy()
                board_after_move.push(move)

                new_val: float = self.min_max(board_after_move, depth + 1, alpha, beta)

                if new_val < val:
                    val = new_val

                if new_val <= alpha:
                    return val

                if new_val < beta:
                    beta = new_val
                    if depth == 0:
                        self.current_best_action = move

            return val

    def run_min_max(self, board: chess.Board):
        self.current_player = board.turn
        self.current_best_action = None
        self.current_max_depth = None
        max_depth = 5

        self.current_player = None

        tic = time.perf_counter()
        for depth in range(max_depth):
            self.current_max_depth = depth + 1
            print(self.current_max_depth)
            eval = self.min_max(board, 0)
            best_action = self.current_best_action
        toc = time.perf_counter()
        print(f"Minmax.preidct() {toc - tic:0.5f}")

        return best_action, eval
