How to internationalize Scenic with the autotools
-------------------------------------------------
In short: ./autogen.sh && make dist will generate the .gmo files.
You need to install Scenic to experience it in French.
LANG=fr_CA.UTF-8 scenic

 * Use gettext, aliased as "_()" in the code.
 * make update-po to parse the code and generate the .po files
 * The translators can edit the po with poedit or vim
 * make update-gmo to build the .gmo files (done by make dist)
 * make install will install them in /usr/local/share/locale/*/*.mo


More generally about the internationalization process
-----------------------------------------------------

Here we describe all the steps involved in producing translations.

So here the steps to produce a translation from scratch:

    We will use the context of creating a new Widget as an example.

    1. Extract the strings to translate from your python script with
       GNU xgettext.
       For Scenic, we have a script called locale/create_gettext_file.sh

           $ xgettext -o your_file.poy your_file.py

    2. If you created many, merge the newly created files
       with msgcat. (it's useless right now in Scenic)

           $ cd pathToLocaleDirectory
           $ msgcat -o scenic.pot input1.pot input2.pot

    3. At some point, you need to edit manually the .pot file to set the
       charset, an other informations. There more info about this at
       http://www.gnu.org/software/gettext/manual/gettext.html#Header-Entry
        * CHARSET should be UTF-8
       You might want to automate this with sed, but there's nothing like a
       human intervention :

           $ sed -i s/CHARSET/utf-8/g scenic.pot

    4. Create one .po file for each language you want to support with msginit::

           $ msginit -l fr_CA -i your_widget.pot

       The resulting file your_widget.po should be created in the directory
       localeDirectory/LL_CC/LC_MESSAGES/. You can add the -o arguments to
       msginit to specified the right place.

    5. If there .po files are already there, you need to update them::

           $ msgmerge en_US/LC_MESSAGES/scenic.po scenic.pot
           $ msgmerge fr_CA/LC_MESSAGES/scenic.po scenic.pot

    6. Translate all the strings in you your_widget.po file for each languages.
       You can do this with a tool like Poedit.

           $ poedit fr_CA/LC_MESSAGES/scenic.po

    6. Convert .po file to the binary format .mo with msgfmt::
       If you use poedit, you can skip this step, since it does it for you.

           $ msgfmt localeDirectory/fr_CA/LC_MESSAGES/your_widget.po

    That's it! Now we need to add how to update the translations with
    the evolution of the code and template.


Interestings notes from http://www.gnu.org/software/gettext
-----------------------------------------------------------

(Why doesn't xgettext create it under the name domainname.pot right away? The answer is: for historical reasons. When xgettext was specified, the distinction between a PO file and PO file template was fuzzy, and the suffix ‘.pot’ wasn't in use at that time.)

The msgmerge program has the purpose of refreshing an already existing lang.po file, by comparing it with a newer package.pot template file, extracted by xgettext out of recent C sources.

