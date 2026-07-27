{
    "patcher": {
        "fileversion": 1,
        "appversion": {
            "major": 9,
            "minor": 1,
            "revision": 4,
            "architecture": "x64",
            "modernui": 1
        },
        "classnamespace": "box",
        "rect": [
            40,
            60,
            860,
            1060
        ],
        "boxes": [
            {
                "box": {
                    "maxclass": "comment",
                    "text": "ortho",
                    "patching_rect": [
                        20,
                        15,
                        200,
                        29
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-1",
                    "fontsize": 20
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "The ortho object generates invented language. It mints a language once from a seed, then draws every word from that same language, so the output holds together as one tongue. The same seed always names the same language.",
                    "patching_rect": [
                        20,
                        50,
                        780,
                        50
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-2",
                    "linecount": 2
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "GENERATE",
                    "patching_rect": [
                        20,
                        110,
                        200,
                        22
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-3",
                    "fontsize": 13
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "bang produces one paragraph. tokens N produces exactly N words with no punctuation. page N produces N paragraphs. section mints a new cast of names and topics without changing the language itself.",
                    "patching_rect": [
                        20,
                        133,
                        780,
                        50
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-4",
                    "linecount": 2
                }
            },
            {
                "box": {
                    "maxclass": "button",
                    "patching_rect": [
                        20,
                        190,
                        24,
                        24
                    ],
                    "outlettype": [
                        "bang"
                    ],
                    "numinlets": 1,
                    "numoutlets": 1,
                    "id": "obj-5"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "tokens 20",
                    "patching_rect": [
                        56,
                        190,
                        72,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-6"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "page 2",
                    "patching_rect": [
                        136,
                        190,
                        54,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-7"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "section",
                    "patching_rect": [
                        198,
                        190,
                        58,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-8"
                }
            },
            {
                "box": {
                    "maxclass": "newobj",
                    "text": "ortho @seed 12345",
                    "patching_rect": [
                        20,
                        232,
                        126,
                        22
                    ],
                    "outlettype": [
                        "",
                        ""
                    ],
                    "numinlets": 1,
                    "numoutlets": 2,
                    "id": "obj-9"
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "words \u2014 left outlet",
                    "patching_rect": [
                        20,
                        268,
                        180,
                        20
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-10"
                }
            },
            {
                "box": {
                    "maxclass": "newobj",
                    "text": "prepend set",
                    "patching_rect": [
                        20,
                        290,
                        76,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 1,
                    "numoutlets": 1,
                    "id": "obj-11"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "",
                    "patching_rect": [
                        20,
                        318,
                        470,
                        78
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-12",
                    "linecount": 5
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "sources \u2014 right outlet",
                    "patching_rect": [
                        510,
                        268,
                        180,
                        20
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-13"
                }
            },
            {
                "box": {
                    "maxclass": "newobj",
                    "text": "prepend set",
                    "patching_rect": [
                        510,
                        290,
                        76,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 1,
                    "numoutlets": 1,
                    "id": "obj-14"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "",
                    "patching_rect": [
                        510,
                        318,
                        290,
                        78
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-15",
                    "linecount": 5
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "SOURCES",
                    "patching_rect": [
                        20,
                        415,
                        200,
                        22
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-16",
                    "fontsize": 13
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "The right outlet reports why each word appeared, one integer per word, lined up position for position with the words on the left. 0 is freshly generated, 1 a function word, 2 a topic (the section's subject), 3 a name (the section's identities), 4 a member of a recurring phrase.",
                    "patching_rect": [
                        20,
                        438,
                        780,
                        66
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-17",
                    "linecount": 3
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "DIALS",
                    "patching_rect": [
                        20,
                        520,
                        200,
                        22
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-18",
                    "fontsize": 13
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "Seven dials, each 0 to 1, all starting at 0. At zero every word is freshly generated. Raise a dial and that kind of word starts recurring. The first four affect every path; the last three add punctuation and are ignored by tokens.",
                    "patching_rect": [
                        20,
                        543,
                        780,
                        50
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-19",
                    "linecount": 2
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "names 0.6",
                    "patching_rect": [
                        20,
                        600,
                        76,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-20"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "topics 0.4",
                    "patching_rect": [
                        101,
                        600,
                        74,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-21"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "phrases 0.5",
                    "patching_rect": [
                        180,
                        600,
                        80,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-22"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "function_words 0.5",
                    "patching_rect": [
                        265,
                        600,
                        126,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-23"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "commas 0.4",
                    "patching_rect": [
                        396,
                        600,
                        80,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-24"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "quotation 0.3",
                    "patching_rect": [
                        481,
                        600,
                        90,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-25"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "scare_quotes 0.2",
                    "patching_rect": [
                        576,
                        600,
                        108,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-26"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "preset 0.5",
                    "patching_rect": [
                        20,
                        632,
                        78,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-27"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "cleardials",
                    "patching_rect": [
                        103,
                        632,
                        76,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-28"
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "preset fills every dial you have not set by hand, so order never matters. cleardials zeroes all seven dials and the preset together.",
                    "patching_rect": [
                        187,
                        632,
                        600,
                        33
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-29",
                    "linecount": 2
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "SHAPING",
                    "patching_rect": [
                        20,
                        690,
                        200,
                        22
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-30",
                    "fontsize": 13
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "These three are not dials. They do not change the language, only how much of it comes out and how long the words run.",
                    "patching_rect": [
                        20,
                        713,
                        780,
                        33
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-31"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "max_letters 6",
                    "patching_rect": [
                        20,
                        755,
                        98,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-32"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "max_words 8",
                    "patching_rect": [
                        123,
                        755,
                        90,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-33"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "sentences 2",
                    "patching_rect": [
                        218,
                        755,
                        86,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-34"
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "PUNCTUATION",
                    "patching_rect": [
                        20,
                        800,
                        200,
                        22
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-35",
                    "fontsize": 13
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "With commas, quotation, or scare_quotes above zero, punctuation is attached to the word it belongs to, so a token can arrive as one symbol carrying a comma. That symbol travels safely through zl, coll, and route. It is only re-read if it lands in a message box, where Max treats a comma as a message separator. Use tokens when you need output guaranteed free of punctuation.",
                    "patching_rect": [
                        20,
                        823,
                        780,
                        83
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-36",
                    "linecount": 3
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "SEED",
                    "patching_rect": [
                        20,
                        920,
                        200,
                        22
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-37",
                    "fontsize": 13
                }
            },
            {
                "box": {
                    "maxclass": "comment",
                    "text": "The seed is the language. Change it and every word afterwards comes from a different tongue. Saved with the patcher, along with every dial.",
                    "patching_rect": [
                        20,
                        943,
                        780,
                        33
                    ],
                    "numinlets": 1,
                    "numoutlets": 0,
                    "id": "obj-38",
                    "linecount": 2
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "seed 1",
                    "patching_rect": [
                        20,
                        985,
                        58,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-39"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "seed 42",
                    "patching_rect": [
                        83,
                        985,
                        64,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-40"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "seed 12345",
                    "patching_rect": [
                        152,
                        985,
                        82,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-41"
                }
            },
            {
                "box": {
                    "maxclass": "message",
                    "text": "seed 4294967295",
                    "patching_rect": [
                        239,
                        985,
                        112,
                        22
                    ],
                    "outlettype": [
                        ""
                    ],
                    "numinlets": 2,
                    "numoutlets": 1,
                    "id": "obj-42"
                }
            }
        ],
        "lines": [
            {
                "patchline": {
                    "source": [
                        "obj-5",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-6",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-7",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-8",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-9",
                        0
                    ],
                    "destination": [
                        "obj-11",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-11",
                        0
                    ],
                    "destination": [
                        "obj-12",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-9",
                        1
                    ],
                    "destination": [
                        "obj-14",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-14",
                        0
                    ],
                    "destination": [
                        "obj-15",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-20",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-21",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-22",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-23",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-24",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-25",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-26",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-27",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-28",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-32",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-33",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-34",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-39",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-40",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-41",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            },
            {
                "patchline": {
                    "source": [
                        "obj-42",
                        0
                    ],
                    "destination": [
                        "obj-9",
                        0
                    ]
                }
            }
        ]
    }
}