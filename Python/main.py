import sys

from ChessBoard import *
from ChessEngine import ChessEngine

pygame.init()

size = width, height = 1000, 1000
black = 0, 0, 0

screen = pygame.display.set_mode(size)
clock = pygame.time.Clock()

chess_board = ChessBoard()
# fen1 = "r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq -"
# fen2 = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1"
#
# chess_board.chess_logic.set_fen(fen2)

game_over = False

human_player = chess.WHITE
ai_player_1 = chess.BLACK
ai_player_2 = None

assert human_player != ai_player_1
assert human_player != ai_player_2
assert ai_player_1 != ai_player_2

model = ChessEngine()

human_move_frame = False

while 1:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        if not game_over:
            if chess_board.chess_logic.turn == human_player:
                human_move_frame = True
                if event.type == pygame.MOUSEBUTTONDOWN:
                    chess_board.on_mouse_button_down(event.pos)

                if event.type == pygame.MOUSEBUTTONUP:
                    chess_board.on_mouse_button_up(event.pos)
                    if chess_board.chess_logic.is_game_over():
                        print(f"{chess_board.chess_logic.result()}")
                        game_over = True

    if not game_over and not human_move_frame:
        if chess_board.chess_logic.turn == ai_player_1:
            move, eval = model.run_engine(chess_board.chess_logic)
            print(f"evaluation: {eval}, move: {move}")
            chess_board.chess_logic.push(move)

        elif chess_board.chess_logic.turn == ai_player_2:
            move, eval = model.run_engine(chess_board.chess_logic)
            print(f"evaluation: {eval}, move: {move}")
            chess_board.chess_logic.push(move)

    chess_board.update_FEN(chess_board.chess_logic.fen())
    human_move_frame = False
    screen.fill(black)
    chess_board.draw_on(screen)
    pygame.display.flip()
    clock.tick(60)
