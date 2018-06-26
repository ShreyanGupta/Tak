#include <algorithm>

#include "util.h"
#include "bitboard.h"
#include "move_generator.h"

#define crBegin STATE=0; switch(STATE) { case 0:
#define crReturn(x) do { STATE=__LINE__; return x; case __LINE__:; } while (0)
#define crFinish STATE=-1; case -1: assert(false); }

namespace Tak {

MoveGenerator::MoveGenerator(BitBoard &board) : 
  board(board) {}

bool MoveGenerator::has_next() {

  crBegin;

  bool has_flat;
  bool has_cap;
  if(is_black(board.current_player)) {
    STATE_curr_stones = board.black_stones;
    has_flat = board.pieces.black_flat != 0;
    has_cap = board.pieces.black_cap != 0;
  }
  else {
    STATE_curr_stones = board.white_stones;
    has_flat = board.pieces.white_flat != 0;
    has_cap = board.pieces.white_cap != 0;
  }

  // Figure out which all places moves are valid
  if(has_flat){
    STATE_move_type.push_back(MoveType::PlaceFlat);
    STATE_move_type.push_back(MoveType::PlaceWall);
  }
  if(has_cap){
    STATE_move_type.push_back(MoveType::PlaceCapstone);
  }

  // Generate place moves
  for(i=0; i<board.height.size(); ++i){
    if(!test_bit(STATE_curr_stones, i)) continue;
    // Set move pos
    move.pos = i;
    for(j=0; j<STATE_move_type.size(); ++j){
      move.move_type = STATE_move_type[j];
      crReturn(true);
    }
  }

  // Should be in reverse order as the get_max_slide function
  STATE_move_type = {
    MoveType::SlideDown,
    MoveType::SlideUp, 
    MoveType::SlideRight, 
    MoveType::SlideLeft, 
  };

  // Generate slide moves
  for(i=0; i<board.height.size(); ++i){
    // If not owned by current_player, continue
    if(!test_bit(STATE_curr_stones, i)) continue;

    // set move pos
    // get informations about slides
    move.pos = i;
    STATE_slides = get_max_slide(i);

    // Iterate over all the slide moves
    for(j=0; j<STATE_move_type.size(); ++j){
      // Set the move type
      move.move_type = STATE_move_type[j];

      // Get the value of numx_drops and numx_pieces
      STATE_num_pieces = std::min((int)board.height[i], size);
      STATE_num_drops = (STATE_slides >> (4*j + 4)) & 0xf;
      // If we can't reach cap_move wall, unset cap_move
      if(STATE_num_pieces < STATE_num_drops+1) reset_bit(STATE_slides, j);
      STATE_num_drops = std::min(STATE_num_drops, STATE_num_pieces);
      
      // Get pointer to appropriate slide_vec
      // Iterate over all slide values
      STATE_slide_vec = &SlideVec::all_slides[STATE_num_drops][STATE_num_pieces];
      for(k=0; k<STATE_slide_vec->size(); ++k){
        move.slide = (*STATE_slide_vec)[k];
        crReturn(true);
      }

      // If cap bit is not set, continue
      if(!test_bit(STATE_slides, j)) continue;

      // Set cap_move
      // Get pointer to appropriate slide_vec for cap moves
      // Iterate over all slide values
      // Unset cap_move
      
      move.cap_move = true;
      STATE_slide_vec = &SlideVec::cap_slides[STATE_num_drops+1][STATE_num_pieces];
      for(k=0; k<STATE_slide_vec->size(); ++k){
        move.slide = (*STATE_slide_vec)[k];
        crReturn(true);
      }
      move.cap_move = false;
    }
  }
  crFinish;
  return false;
}

// Returns the slides possible for each direction
// ret & 0xf gives the 4 cap_able bits
// (ret & 0xf0) to (ret & 0xf0000) gives the max slide values
// for directions L, R, U, D respectively
Bit MoveGenerator::get_max_slide(s_int pos){
  Bit capable = 0;
  Bit slides = 0;

  // In the sequence left, right, up, down
  vector<int> directions = {-1, +1, size, -size};
  vector<Bit> boundaries = {Bits::L, Bits::R, Bits::U, Bits::D};

  for(int i=0; i<4; ++i){
    // If pos lies on the boundry, continue
    capable <<= 1;
    auto temp_pos = pos;
    while(true){
      // break if we hit the boundary
      // note that there is no wall on boundary
      if((1 << temp_pos) & boundaries[i]) break;
      // Increment the position
      temp_pos += directions[i];
      // break if we hit the wall, set capable
      // note that the wall may be on the boundary
      if((1 << temp_pos) & board.wall_stones){
        ++capable;
        break;
      }
      // Only increment slides if neither on boundry or wall
      ++slides;
    }
    slides <<= 4;
  }
  return slides | capable;
}

} // namespace Tak