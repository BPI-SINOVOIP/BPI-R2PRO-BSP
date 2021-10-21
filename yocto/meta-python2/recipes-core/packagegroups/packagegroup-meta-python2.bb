SUMMARY = "Meta-oe ptest packagegroups"

inherit packagegroup

PROVIDES = "${PACKAGES}"
PACKAGES = ' \
    packagegroup-meta-python2 \
'

RDEPENDS_packagegroup-meta-python2 = "\
    packagegroup-meta-python2-extended \
    packagegroup-meta-python2-connectivity \
"

RDEPENDS_packagegroup-meta-python2 = "\
    python-psutil python-certifi python-flask python-pyroute2 python-pyopenssl python-pylint \
    python-semver python-wrapt python-networkx python-behave python-dominate python-flask-user \
    python-attrs python-humanize python-six python-flask-login python-zopeinterface python-sijax \
    python-pyinotify python-stevedore python-pyjwt python-webdav python-twisted python-flask-sijax \
    python-functools32 python-javaobj-py3 python-pygpgme python-future python-attr \
    python-flask-xstatic python-m2crypto python-hyperlink python-imaging python-idna python-jinja2 \
    python-can python-flask-bcrypt python-requests python-paste python-flask-script python-serpent \
    python-cryptography python-pysmi python-xlrd python-appdirs python-jsonpatch python-bcrypt \
    python-ndg-httpsclient python-pytest python-linecache2 python-visitor python-backports-abc \
    python-setuptools-scm python-evdev python-pyjks python-jsonpointer python-cheetah python-gevent \
    python-smbus python-sqlalchemy python-scrypt python-werkzeug python-anyjson python-pexpect \
    python-robotframework-seriallibrary python-pyalsaaudio python-pytest-helpers-namespace \
    python-alembic python-flask-pymongo python-slip-dbus python-pydbus python-automat python-rfc3987 \
    python-tzlocal python-backports-ssl python-subprocess32 python-asn1crypto python-pybind11 \
    python-ptyprocess python-babel python-passlib python-sdnotify \
    python-lazy-object-proxy python-cryptography-vectors python-crcmod python-pyusb python-vobject \
    python-webcolors python-pyparsing python-beautifulsoup4 python-cffi python-tornado-redis \
    python-itsdangerous python-pyasn1-modules python-netaddr python-vcversioner \
    python-sh python-greenlet python-paho-mqtt python-traceback2 python-gdata python-dbusmock \
    python-whoosh python-lockfile python-isort python-wtforms python-feedparser python-flask-restful \
    python-pysnmp python-flask-babel python-pytest-tempdir python-flask-nav python-pyzmq python-pyyaml \
    python-protobuf python-pluggy python-jsonschema python-msgpack \
    python-periphery python-pint python-pycryptodomex python-yappi python-pycrypto python-pretend \
    python-pyserial python-pyiface python-docutils python-grpcio-tools python-django-south \
    python-backports-functools-lru-cache python-py python-click python-flask-migrate \
    python-pyudev python-pystache python-blinker python-prompt-toolkit python-lxml \
    python-unidiff python-inflection python-twofish python-prettytable python-webencodings \
    python-mock python-pyexpect python-dnspython python-pysocks python-pynetlinux \
    python-daemon python-djangorestframework python-typing python-monotonic python-sparts \
    python-enum34 python-pyperclip python-flask-uploads python-pbr python-parse python-pyflakes \
    python-pyhamcrest python-mako python-incremental python-tornado python-xstatic-font-awesome \
    python-cmd2 python-strict-rfc3339 python-pycodestyle python-xstatic python-snakefood \
    python-pybluez python-flask-navigation python-pyfirmata python-pymongo python-pysqlite \
    python-progress python-flask-sqlalchemy python-pymisp python-pip python-ujson python-ply \
    python-pep8 python-dateutil python-pycparser python-daemonize python-astroid python-pyrex \
    python-markupsafe python-pytest-runner python-grpcio python-mccabe python-pytz python-selectors34 \
    python-cython python-chardet python-editor python-flask-bootstrap python-html5lib \
    python-singledispatch python-redis python-flask-mail python-funcsigs python-snimpy python-pyasn1 \
    python-decorator python-urllib3 python-feedformatter python-iso8601 \
    python-numeric python-robotframework python-django python-simplejson python-wcwidth \
    python-configparser python-epydoc python-intervals python-speaklater \
    python-aws-iot-device-sdk-python python-constantly python-bitarray python-flask-wtf \
    python-parse-type python-ipaddress python-dbus python-cpuset python-distutils-extra \
    python-futures python-jsmin python-pygobject python-pytoml python-six python-which \
    python-netifaces python-configargparse python-sqlparse python-soupsieve python-wrapt \
    python-deprecated python-booleanpy python-docker-pycreds python-websocket-client \
    python-docker \
    ${@bb.utils.contains("DISTRO_FEATURES", "pam", "python-pam pamela", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "systemd", "python-systemd", "", d)} \
"

RDEPENDS_packagegroup-meta-python2-extended = "\
    python-cson \
    python-pyephem \
    python-pyparted \
    python-pywbem \
"

RDEPENDS_packagegroup-meta-python2-connectivity = "\
    python-gsocketpool \
    python-mprpc \
    python-networkmanager \
    python-pyconnman \
    python-pyro4 \
    python-pytun \
    python-thrift \
    python-txws \
"

RDEPENDS_packagegroup-meta-python2-ptest = "\
    python-booleanpy-ptest \
    python-cryptography-ptest \
    python-pygpgme-ptest \
"

EXCLUDE_FROM_WORLD = "1"
