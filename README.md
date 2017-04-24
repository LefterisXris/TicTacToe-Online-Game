Εργασία στα Δίκτυα. Απλοϊκός on-line game Server Τρίλιζας. Απρίλιος 2017

Σημείωση 1: Ο πηγαίος κώδικας καθώς και τα εκτελέσιμα αρχεία βρίσκονται στον φάκελο 
	    make_win32/Computer Networks and Internets.

Σημείωση 2: Επειδή δούλεψα πάνω στον chatserver και chatclient έχουν μείνει τα ονόματα αυτά
            στο project. Τα εκτελέσιμα βρίσκονται στους φακέλους chatserver/Release και 
	    chatclient/Release με ονόματα TrrilizaS.exe και TrilizaC.exe αντίστοιχα.

Οδηγίες χρήσης:

1) Για την έναρξη του παιχνιδιού, πρώτα απαιτείται η εκκίνηση του Server. Για να
   ανοίξετε τον Server εκτελέστε (προτιμότερο από γραμμή εντολών) το αρχείο TrilizaS.exe.
   Αμέσως θα ξεκινήσει να τρέχει ο Server του παιχνιδιού ο οποίος θα περιμένει να 
   συνδεθούν οι παίχτες. Ο Server ακούει στην πόρτα 20000.

2) Εκτελέστε το αρχείο TrilizaC.exe. Ξεκινάει ένα πρόγραμμα πελάτη που αποτελεί τον
   πρώτο παίχτη του παιχνιδιού. Ο πελάτης αυτός συνδέεται στον Server του οποίου
   τα στοιχεία, διαβάζει από το αρχείο triliza.conf (υπάρχει στον φάκελο chatclient/Release 
   και περιέχει τη διεύθυνση του Server και την πόρτα που ακούει), και περιμένει και 
   τον δεύτερο παίχτη.

3) Εκτελέστε και πάλι το αρχείο TrilizaC.exe ώστε να ξεκινήσει και ο δεύτερος παίχτης.

4) Το παιχνίδι πλέον είναι έτοιμο να ξεκινήσει. Ο παίχτης που συνδέθηκε πρώτος, επιλέγει
   την πρώτη κίνηση και παίρνει το σύμβολο X, ενώ ο δέυτερος το O.

5) Οι κινήσεις που μπορεί να κάνει ο κάθε παίχτης είναι οι επιλογή αριθμού από 1 έως 9 
   και αντιστοιχούν στα κελιά της τρίλιζας ξεκινώντας από το πάνω αριστερά κελί και 
   συνεχίζει προς τα δεξιά και μετά από το αριστερότερο κελί της κάτω γραμμής, κόκ.

6) Κάθε παίχτης έχει στη διάθεση του 20 δευτερόλεπτα να αποφασίσει την κίνησή του. Αν 
   τα ξεπεράσει τότε αυτόματα χάνει.

7) Αν κάποιος παίχτης κάνει 3 λανθασμένες επιλογές, τότε αυτόματα χάνει.

8) Το παιχνίδι τελειώνει είτε αν γίνει κάτι από το (6) ή το (7), είτε αν κάποιος 
   παίχτης σχηματίσει τρίλιζα, είτε αν συμπληρωθούν όλα τα κελιά της τρίλιζας οπότε
   θα έχουμε ισοπαλία.


Σημείωση: Για να τρέξει ο κώδικας, πρέπει να φορτωθεί στο Visual Studio το project (COMPUTER NETWORKS AND IN~3f.sln) και να γίνει Build των δυο επιμέρους project που θα εμφανιστούν στο Solution Explorer (TrilizaS και TrilizaC).

# Screenshots

### 1 Initialization
![Initialization](Initialization.png)
### 2 Some moves
![Moves](Moves.png)
### 3 Game Over
![Win](Win.png)