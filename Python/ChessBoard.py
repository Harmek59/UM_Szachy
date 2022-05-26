import chess
import pygame


class PieceSprite(pygame.sprite.Sprite):
    def __init__(self, texture_path: str, image_size: int):
        super().__init__()
        self.image = pygame.image.load(texture_path).convert_alpha()
        self.image = pygame.transform.scale(self.image, (image_size, image_size))
        self.rect = self.image.get_rect()


class BoardSprite:
    def __init__(self, board_size: int, board_center: (int, int), board_texture_path: str = ''):
        self.black_color = (81, 42, 42)
        self.white_color = (124, 76, 62)
        self.tile_size = board_size / 8
        self.image = pygame.Surface((board_size, board_size))
        self.rect = self.image.get_rect()
        self.rect.center = (board_center[0], board_center[1])

        if len(board_texture_path):
            texture = pygame.image.load(board_texture_path).convert_alpha()
            texture = pygame.transform.scale(texture, (board_size, board_size))
            tex_rect = texture.get_rect()
            self.image.blit(texture, tex_rect)
        else:
            tile_rectangle = pygame.rect.Rect(0, 0, self.tile_size, self.tile_size)
            for x in range(8):
                for y in range(8):
                    tile_color = ''
                    if (x + y) % 2:
                        tile_color = self.black_color
                    else:
                        tile_color = self.white_color

                    tile_rectangle.topleft = pygame.Vector2(self.tile_size * x, self.tile_size * y)
                    pygame.draw.rect(self.image, tile_color, tile_rectangle)


class ChessBoard:
    def __init__(self):
        self.board_size = 960
        self.piece_size = 960 // 8
        self.board_center = (1280 / 2, 1000 / 2)
        self.load_chess_set()
        self.board_sprite = BoardSprite(self.board_size, self.board_center)
        self.tile_size = self.board_size / 8

        self.FEN_cache = ''

        self.picked_up_piece_form_tile = pygame.Vector2(-1, -1)
        self.picked_piece = None
        self.piece_move = None

        self.chess_logic = chess.Board()
        self.update_FEN(self.chess_logic.fen())

    def draw_on(self, surface: pygame.surface.Surface) -> None:
        surface.blit(self.board_sprite.image, self.board_sprite.rect)
        if len(self.FEN_cache):
            self.draw_pieces_on(surface)

    def calculate_tile_from_mouse_pos(self, mouse_position: pygame.Vector2):
        mouse_position_on_board = (
            mouse_position[0] - self.board_sprite.rect.topleft[0],
            mouse_position[1] - self.board_sprite.rect.topleft[1])
        if mouse_position_on_board[0] >= self.board_size or mouse_position_on_board[1] >= self.board_size:
            return None
        x = int(mouse_position_on_board[0] / self.tile_size)
        y = int(mouse_position_on_board[1] / self.tile_size)
        return (x, y)

    def on_mouse_button_up(self, mouse_position: pygame.Vector2):
        tile = self.calculate_tile_from_mouse_pos(mouse_position)
        if tile:
            (x, y) = tile
            self.drop_piece_on(x, y)

    def on_mouse_button_down(self, mouse_position: pygame.Vector2):
        tile = self.calculate_tile_from_mouse_pos(mouse_position)
        if tile:
            (x, y) = tile
            self.pick_up_piece_from(x, y)

    def pick_up_piece_from(self, tile_x: int, tile_y: int):
        self.picked_piece = self.chess_logic.piece_at((7 - tile_y) * 8 + tile_x)
        if not self.picked_piece:
            return
        if (self.chess_logic.turn and self.picked_piece.color) or (
                not self.chess_logic.turn and not self.picked_piece.color):
            self.picked_up_piece_form_tile = pygame.Vector2(tile_x, tile_y)
            if self.picked_piece:
                self.piece_move = chess.Move(chess.Square((7 - tile_y) * 8 + tile_x), chess.Square(0))
        else:
            self.picked_piece = None

    def drop_piece_on(self, tile_x: int, tile_y: int):
        if self.picked_piece:
            if self.picked_piece == chess.Piece(chess.PAWN, chess.WHITE) and tile_y == 0:
                self.piece_move = chess.Move(self.piece_move.from_square, chess.Square((7 - tile_y) * 8 + tile_x),
                                             chess.QUEEN)
            elif self.picked_piece == chess.Piece(chess.PAWN, chess.BLACK) and tile_y == 7:
                self.piece_move = chess.Move(self.piece_move.from_square, chess.Square((7 - tile_y) * 8 + tile_x),
                                             chess.QUEEN)
            else:
                self.piece_move.to_square = chess.Square((7 - tile_y) * 8 + tile_x)
            if self.piece_move.from_square != self.piece_move.to_square and self.piece_move in self.chess_logic.legal_moves:
                self.chess_logic.push(self.piece_move)
                self.piece_move = None
        self.picked_up_piece_form_tile = pygame.Vector2(-1, -1)
        self.picked_piece = None

    def load_chess_set(self) -> None:
        pice_size_for_set = self.piece_size - 20
        path_to_pieces = '1x/'
        self.piece_bishop_black = PieceSprite(path_to_pieces + 'b_bishop_1x.png', pice_size_for_set)
        self.piece_bishop_white = PieceSprite(path_to_pieces + 'w_bishop_1x.png', pice_size_for_set)
        self.piece_king_black = PieceSprite(path_to_pieces + 'b_king_1x.png', pice_size_for_set)
        self.piece_king_white = PieceSprite(path_to_pieces + 'w_king_1x.png', pice_size_for_set)
        self.piece_knight_black = PieceSprite(path_to_pieces + 'b_knight_1x.png', pice_size_for_set)
        self.piece_knight_white = PieceSprite(path_to_pieces + 'w_knight_1x.png', pice_size_for_set)
        self.piece_pawn_black = PieceSprite(path_to_pieces + 'b_pawn_1x.png', pice_size_for_set - 10)
        self.piece_pawn_white = PieceSprite(path_to_pieces + 'w_pawn_1x.png', pice_size_for_set - 10)
        self.piece_queen_black = PieceSprite(path_to_pieces + 'b_queen_1x.png', pice_size_for_set)
        self.piece_queen_white = PieceSprite(path_to_pieces + 'w_queen_1x.png', pice_size_for_set)
        self.piece_rook_black = PieceSprite(path_to_pieces + 'b_rook_1x.png', pice_size_for_set)
        self.piece_rook_white = PieceSprite(path_to_pieces + 'w_rook_1x.png', pice_size_for_set)

    def get_tile_center_position(self, x: int, y: int) -> pygame.Vector2:
        return pygame.Vector2((x + 0.5) * self.tile_size, (y + 0.5) * self.tile_size) + self.board_sprite.rect.topleft

    def update_FEN(self, edp: str):
        self.FEN_cache = edp.split(' ')[0]  # take only the part, which shows pieces positions

    def draw_piece_on(self, surface: pygame.surface.Surface, piece: PieceSprite, tile_x: int, tile_y: int) -> None:
        rect = piece.rect
        if tile_x == self.picked_up_piece_form_tile[0] and tile_y == self.picked_up_piece_form_tile[
            1] and self.picked_piece:
            rect.center = pygame.mouse.get_pos()
        else:
            rect.center = self.get_tile_center_position(tile_x, tile_y)
        surface.blit(piece.image, rect)

    def draw_pieces_on(self, surface: pygame.surface.Surface):
        x = 0
        y = 0
        for c in self.FEN_cache:
            if c == '/':
                x = 0
                y = y + 1
            elif c == 'R':
                self.draw_piece_on(surface, self.piece_rook_white, x, y)
            elif c == 'N':
                self.draw_piece_on(surface, self.piece_knight_white, x, y)
            elif c == 'B':
                self.draw_piece_on(surface, self.piece_bishop_white, x, y)
            elif c == 'Q':
                self.draw_piece_on(surface, self.piece_queen_white, x, y)
            elif c == 'K':
                self.draw_piece_on(surface, self.piece_king_white, x, y)
            elif c == 'P':
                self.draw_piece_on(surface, self.piece_pawn_white, x, y)
            elif c == 'r':
                self.draw_piece_on(surface, self.piece_rook_black, x, y)
            elif c == 'n':
                self.draw_piece_on(surface, self.piece_knight_black, x, y)
            elif c == 'b':
                self.draw_piece_on(surface, self.piece_bishop_black, x, y)
            elif c == 'q':
                self.draw_piece_on(surface, self.piece_queen_black, x, y)
            elif c == 'k':
                self.draw_piece_on(surface, self.piece_king_black, x, y)
            elif c == 'p':
                self.draw_piece_on(surface, self.piece_pawn_black, x, y)
            else:
                number_to_skip = int(c)
                x = (x + number_to_skip) % 8

            if c.isalpha():
                x = x + 1
