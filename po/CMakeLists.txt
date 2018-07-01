find_package(Gettext REQUIRED)

gettext_process_po_files(vi ALL PO_FILES vi.po)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vi.gmo"
    DESTINATION "${LOCALEDIR}/vi/LC_MESSAGES"
    RENAME "${PROJECT_NAME}.mo")

