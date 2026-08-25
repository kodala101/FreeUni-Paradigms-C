## დავალების ატვირთვა
დავალება უნდა ატვირთოთ თქვენს პერსონალურ Github Classroom-ის რეპოზიტორიაში.

## კომპილაცია
```sh
make
```

## ტესტებისთვის საჭირო data ფაილები
`make` ის პირველი გაშვება ავტომატურად შექმნის data დირექტორიას ტესტებისთვის საჭირო ფაილებით.  
თუ რატომღაც ეს ფაილები "დაგიზიანდათ", მათი თავიდან ჩამოტვირთვისთვის გაუშვით:
```sh
rm -rf data/
make data
```

## ტესტირება
```sh
./vector-test
./hashset-test
```

`vector-test` და `hashset-test` აპლიკაციებს ეკრანზე გამოაქვს თუ როგორ იქცევა თქვენი ვექტორის და ჰეშსეტის იმპლემენტაცია. რომელიც შეგიძლიათ შეადაროთ `sample-output-vector.txt` და `sample-output-hashset.txt` ფაილებს რათა დარწმუნდეთ თქვენი იმპლემენტაციის სისწორეში.
```sh
./vector-test | diff sample-output-vector.txt -
./hashset-test | diff sample-output-hashset.txt -
```

## Thesaurus
The thesaurus-lookup.c and streaktokenizer.c files, when
compiled against fully operational versions of vector and hashset,
contribute to an application called thesaurus-lookup, which is
this C-string intense application that loads a thesaurus into
a hashset (where synonym sets are stored in vectors).  You don't
need to do anything for this extra thesaurus-lookup application.
I just decided to include it with the starter files so you have
a much more sophisticated test application to exercise your implementations.
It also provides a good amount of sample code for you to read over
while working on next week's Assignment 4, which requires you
use the hashset and the vector to build an index of hundreds of online
news articles (with real networking!)
