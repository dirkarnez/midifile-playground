#include "MidiFile.h"
#include "Options.h"
#include <random>
#include <iostream>

using namespace std;
using namespace smf;

int main(int argc, char** argv) {
   Options options;
   options.define("n|note-count=i:10", "How many notes to randomly play");
   options.define("o|output-file=s",   "Output filename (stdout if none)");
   options.define("i|instrument=i:0",  "General MIDI instrument number");
   options.define("x|hex=b",           "Hex byte-code output");
   options.process(argc, argv);

   {
      random_device rd;
      mt19937 mt(rd());
      uniform_int_distribution<int> starttime(0, 100);
      uniform_int_distribution<int> duration(1, 8);
      uniform_int_distribution<int> pitch(36, 84);
      uniform_int_distribution<int> velocity(40, 100);

      MidiFile midifile;
      int track   = 0;
      int channel = 0;
      int instrument   = options.getInteger("instrument");
      midifile.addTimbre(track, 0, channel, instrument);

      int tpq     = midifile.getTPQ();
      int count   = options.getInteger("note-count");
      for (int i=0; i<count; i++) {
         int starttick = int(starttime(mt) / 4.0 * tpq);
         int key       = pitch(mt);
         int endtick   = starttick + int(duration(mt) / 4.0 * tpq);
         midifile.addNoteOn (track, starttick, channel, key, velocity(mt));
         midifile.addNoteOff(track, endtick,   channel, key);
      }

      midifile.sortTracks();  // Need to sort tracks since added events are
                              // appended to track in random tick order.
      string filename = options.getString("output-file");
      if (filename.empty()) {
         if (options.getBoolean("hex")) midifile.writeHex(cout);
         else cout << midifile;
      } else
         midifile.write(filename);

   }

   {
      // edit the output file, set all the notes to have velocity 127
      string filename = options.getString("output-file");
      if (filename.empty()) {
         return -1;
      }

      MidiFile midifile;
      midifile.read(filename);

      for (int i=0; i<midifile[0].getSize(); i++) {
         if (midifile[0][i].getVelocity() > 0) {
            midifile[0][i].setVelocity(127);
         }
      }

      midifile.write(filename + ".v2.mid");
   }

   return 0;
}